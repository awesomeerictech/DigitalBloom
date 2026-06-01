#include "LibArchiveExtractor.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QUuid>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>

#include <archive.h>
#include <archive_entry.h>

bool LibArchiveExtractor::extractArchiveFromQrc(const QString &qrcFilePath, const QString &destinationDir) {
    QFile file(qrcFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open QRC archive:" << qrcFilePath;
        return false;
    }

    QByteArray archiveData = file.readAll();
    file.close();

    struct archive *a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_xz(a);

    struct archive *ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                                         ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);

    struct archive_entry *entry;
    int r;

    r = archive_read_open_memory(a, archiveData.data(), archiveData.size());
    if (r != ARCHIVE_OK) {
        qWarning() << "archive_read_open_memory() failed:" << archive_error_string(a);
        archive_read_free(a);
        archive_write_free(ext);
        return false;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        QString fullPath = QDir(destinationDir).filePath(archive_entry_pathname(entry));
        archive_entry_set_pathname(entry, fullPath.toUtf8().constData());

        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            qWarning() << "archive_write_header() failed:" << archive_error_string(ext);
        } else {
            const void *buff;
            size_t size;
            la_int64_t offset;

            while (true) {
                r = archive_read_data_block(a, &buff, &size, &offset);
                if (r == ARCHIVE_EOF) break;
                if (r < ARCHIVE_OK) {
                    qWarning() << "archive_read_data_block() failed:" << archive_error_string(a);
                    break;
                }
                r = archive_write_data_block(ext, buff, size, offset);
                if (r < ARCHIVE_OK) {
                    qWarning() << "archive_write_data_block() failed:" << archive_error_string(ext);
                    break;
                }
            }
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);

    return true;
}


bool LibArchiveExtractor::extractTarXzFromBuffer(const QByteArray &data, const QString &destinationDir) {
    struct archive *a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_xz(a);

    if (archive_read_open_memory(a, data.constData(), data.size()) != ARCHIVE_OK) {
        qWarning() << "archive_read_open_memory failed:" << archive_error_string(a);
        archive_read_free(a);
        return false;
    }

    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        QString fullPath = destinationDir + "/" + archive_entry_pathname(entry);
        QFileInfo fi(fullPath);
        QDir().mkpath(fi.path());

        if (archive_entry_filetype(entry) == AE_IFDIR) {
            QDir().mkpath(fullPath);
        } else {
            QFile file(fullPath);
            if (!file.open(QIODevice::WriteOnly)) {
                qWarning() << "Failed to write file:" << fullPath;
                archive_read_free(a);
                return false;
            }

            const void *buff;
            size_t size;
            la_int64_t offset;

            while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
                file.write(reinterpret_cast<const char *>(buff), static_cast<qint64>(size));
            }

            file.close();
        }

        archive_entry_set_perm(entry, 0755);  // Optional: set permissions
    }

    archive_read_close(a);
    archive_read_free(a);
    return true;

}

bool LibArchiveExtractor::extractTarXzFromMemory(const void* data, size_t size, const QString& destination) {
    struct archive* a = archive_read_new();
    struct archive* ext = archive_write_disk_new();
    struct archive_entry* entry = nullptr;
    int r = ARCHIVE_OK;

    archive_read_support_filter_xz(a);
    archive_read_support_format_tar(a);

    r = archive_read_open_memory(a, data, size);
    if (r != ARCHIVE_OK) {
        qWarning() << "archive_read_open_memory failed:" << archive_error_string(a);
        archive_read_free(a);
        archive_write_free(ext);
        return false;
    }

    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
    archive_write_disk_set_standard_lookup(ext);

    // Debug: show a few entry names so we can detect top-level folder issues
    int debugEntryCount = 0;
    const int maxDebugEntries = 8;

    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char* entryPathC = archive_entry_pathname(entry);
        if (entryPathC)
            qInfo() << "archive entry:" << QString::fromUtf8(entryPathC);
        if (++debugEntryCount >= maxDebugEntries)
            qInfo() << "(... more entries ...)";

        // Build output path by concatenating destination and entry pathname
        const char* currentFile = archive_entry_pathname(entry);
        if (!currentFile) {
            qWarning() << "archive entry has no pathname; skipping";
            archive_read_data_skip(a);
            continue;
        }

        QString fullOutputPath = destination + "/" + QString::fromUtf8(currentFile);

        // Ensure parent directories exist
        QFileInfo fi(fullOutputPath);
        if (!QDir().mkpath(fi.path())) {
            qWarning() << "Failed to create directory:" << fi.path();
            // continue attempt extraction; but still warn
        }

        // Set pathname to the destination path for extraction to disk
        archive_entry_set_pathname(entry, fullOutputPath.toUtf8().constData());

        // Write header to disk
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            qWarning() << "archive_write_header failed:" << archive_error_string(ext)
            << " for entry:" << fullOutputPath;
            // Try to continue to next header (do not abort immediately)
            // but still attempt to skip data blocks for this entry
            archive_read_data_skip(a);
            continue;
        } else {
            // If it's a regular file, copy its data blocks
            if (archive_entry_size(entry) > 0) {
                const void* buff;
                size_t blockSize;
                la_int64_t offset;

                while (true) {
                    r = archive_read_data_block(a, &buff, &blockSize, &offset);
                    if (r == ARCHIVE_EOF) {
                        break;
                    }
                    if (r < ARCHIVE_OK) {
                        qWarning() << "archive_read_data_block error:" << archive_error_string(a)
                        << " for entry:" << fullOutputPath;
                        break;
                    }

                    r = archive_write_data_block(ext, buff, blockSize, offset);
                    if (r < ARCHIVE_OK) {
                        qWarning() << "archive_write_data_block error:" << archive_error_string(ext)
                        << " for entry:" << fullOutputPath;
                        break;
                    }
                }
            }
        }

        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
            qWarning() << "archive_write_finish_entry failed:" << archive_error_string(ext)
            << " after entry:" << fullOutputPath;
            // continue processing other entries
        }
    }

    if (r != ARCHIVE_OK && r != ARCHIVE_EOF) {
        // An error occurred while reading headers
        qWarning() << "archive_read_next_header failed:" << archive_error_string(a);
        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);
        return false;
    }

    // Clean up
    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);

    qInfo() << "Extraction completed to:" << destination;
    return true;
}


QJsonObject LibArchiveExtractor::extractAndProcessTarXzFile(const QString &sourceFilePath,
                                                            const QString &destinationDir,
                                                            const QString &username,
                                                            const QString &baseName,
                                                            const QString &category) {
    QFile archiveFile(sourceFilePath);
    if (!archiveFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open archive file:" << sourceFilePath;
        return QJsonObject();
    }

    // 1. Prepare libarchive structures
    struct archive *a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_xz(a);

    int r = archive_read_open_fd(a, archiveFile.handle(), 10240); // Open file descriptor for reading
    if (r != ARCHIVE_OK) {
        qWarning() << "archive_read_open_fd failed:" << archive_error_string(a);
        archive_read_free(a);
        return QJsonObject();
    }

    // Prepare destination directory
    QDir destination(destinationDir);
    if (!destination.mkpath(".")) {
        qWarning() << "Failed to create destination directory:" << destinationDir;
        archive_read_close(a);
        archive_read_free(a);
        return QJsonObject();
    }

    // JSON structure preparation
    QJsonObject extractedData;
    QJsonObject categoriesData;

    // Arrays for 'extracted_data'
    QJsonArray itemsIdArray, idsIdArray, uuidArray, mediaFileArray, nameArray, categoryArray,
        usernameArray, sizeOrQuantityArray, standardizedArray, standardizedAvatarArray,
        manufacturerArray, priceArray, descriptionArray, createdAtArray;

    // Arrays for 'categories_data'
    QJsonArray categoryIdArray, categoryNameArray, categoryUsernameArray, categoryCreatedAtArray, categoryIdsIdArray;

    struct archive_entry *entry;

    // Get primary UUIDs and timestamps
    QUuid primaryItemsId = QUuid::createUuid();
    QUuid primaryIdsId = QUuid::createUuid();
    QUuid primaryUuid = QUuid::createUuid();
    QString createdAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // JSON 'data' object (single item for the primary record)
    QJsonObject dataObject;
    dataObject["items_id"] = primaryItemsId.toString(QUuid::WithoutBraces);
    dataObject["ids_id"] = primaryIdsId.toString(QUuid::WithoutBraces);
    dataObject["uuid"] = primaryUuid.toString(QUuid::WithoutBraces);
    dataObject["username"] = username;
    dataObject["name"] = baseName;
    dataObject["category"] = category;
    dataObject["mediafile"] = ""; // Will be updated later if needed
    dataObject["sizeorquantity"] = "N/A"; // Placeholder
    dataObject["standardized"] = "later"; // Placeholder
    dataObject["standardizedavatar"] = "later"; // Placeholder
    dataObject["manufacturer"] = "later"; // Placeholder
    dataObject["price"] = "N/A"; // Placeholder
    dataObject["description"] = "Extracted from " + QFileInfo(sourceFilePath).fileName();
    dataObject["created_at"] = createdAt;

    // Category record for 'categories' table
    QJsonObject categoryObject;
    categoryObject["categories_id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    categoryObject["name"] = category;
    categoryObject["username"] = username;
    categoryObject["created_at"] = createdAt;
    categoryObject["ids_id"] = primaryIdsId.toString(QUuid::WithoutBraces); // Reusing ids_id

    // Category JSON arrays (single item)
    categoryIdArray.append(categoryObject["categories_id"].toString());
    categoryNameArray.append(categoryObject["name"].toString());
    categoryUsernameArray.append(categoryObject["username"].toString());
    categoryCreatedAtArray.append(categoryObject["created_at"].toString());
    categoryIdsIdArray.append(categoryObject["ids_id"].toString());

    // 2. Extract and Process Files
    int fileIndex = 0;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        if (archive_entry_filetype(entry) != AE_IFREG) {
            // Skip non-regular files (directories, links, etc.)
            archive_read_data_skip(a);
            continue;
        }

        QString originalFileName = QString::fromUtf8(archive_entry_pathname(entry));
        QFileInfo originalFileInfo(originalFileName);

        // Generate new file name (UUID + original extension)
        QUuid fileUuid = QUuid::createUuid();
        QString newFileName = fileUuid.toString(QUuid::WithoutBraces) + "." + originalFileInfo.suffix();
        QString fullPath = destination.filePath(newFileName);

        // Write file to disk
        QFile outputFile(fullPath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            qWarning() << "Failed to write file:" << fullPath;
            archive_read_data_skip(a); // Skip data block to continue to next entry
            continue;
        }

        const void *buff;
        size_t size;
        la_int64_t offset;

        while (true) {
            r = archive_read_data_block(a, &buff, &size, &offset);
            if (r == ARCHIVE_EOF) break;
            if (r < ARCHIVE_OK) {
                qWarning() << "archive_read_data_block() failed:" << archive_error_string(a);
                break;
            }
            outputFile.write(reinterpret_cast<const char *>(buff), static_cast<qint64>(size));
        }
        outputFile.close();

        archive_write_finish_entry(a); // Mark entry finished in read-only process

        if (r < ARCHIVE_OK && r != ARCHIVE_EOF) {
            qWarning() << "Error during file extraction/write:" << archive_error_string(a);
            continue; // Move to next entry
        }

        // 3. Populate JSON arrays for "extracted_data"
        QUuid entryItemsId = QUuid::createUuid();
        QUuid entryUuid = QUuid::createUuid();

        itemsIdArray.append(entryItemsId.toString(QUuid::WithoutBraces));
        idsIdArray.append(primaryIdsId.toString(QUuid::WithoutBraces)); // Reuse primary ids_id
        uuidArray.append(entryUuid.toString(QUuid::WithoutBraces));
        mediaFileArray.append(newFileName); // The new UUID-based name

        // Populate other fields for items table
        QString entryName = baseName + QString::number(++fileIndex);
        nameArray.append(entryName);
        categoryArray.append(category);
        usernameArray.append(username);
        sizeOrQuantityArray.append("N/A");
        standardizedArray.append("later");
        standardizedAvatarArray.append("later");
        manufacturerArray.append("later");
        priceArray.append("N/A");
        descriptionArray.append("File extracted: " + originalFileName);
        createdAtArray.append(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

        // Update the primary 'data' object's mediafile field with the first extracted file
        if (fileIndex == 1) {
            dataObject["mediafile"] = newFileName;
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archiveFile.close(); // Close the QFile handle

    // 4. Assemble final JSON object

    // "extracted_data" object for 'items' table
    QJsonObject extractedItemsData;
    extractedItemsData["items_id"] = itemsIdArray;
    extractedItemsData["ids_id"] = idsIdArray;
    extractedItemsData["username"] = usernameArray;
    extractedItemsData["name"] = nameArray;
    extractedItemsData["uuid"] = uuidArray;
    extractedItemsData["category"] = categoryArray;
    extractedItemsData["mediafile"] = mediaFileArray;
    extractedItemsData["sizeorquantity"] = sizeOrQuantityArray;
    extractedItemsData["standardized"] = standardizedArray;
    extractedItemsData["standardizedavatar"] = standardizedAvatarArray;
    extractedItemsData["manufacturer"] = manufacturerArray;
    extractedItemsData["price"] = priceArray;
    extractedItemsData["description"] = descriptionArray;
    extractedItemsData["created_at"] = createdAtArray;

    // "extracted_data" object for 'categories' table
    QJsonObject extractedCategoriesData;
    extractedCategoriesData["categories_id"] = categoryIdArray;
    extractedCategoriesData["name"] = categoryNameArray;
    extractedCategoriesData["username"] = categoryUsernameArray;
    extractedCategoriesData["created_at"] = categoryCreatedAtArray;
    extractedCategoriesData["ids_id"] = categoryIdsIdArray;

    // Final JSON Root Object
    QJsonObject rootObject;
    rootObject["request_id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    rootObject["action"] = "insert";
    rootObject["entity"] = "items";

    // Placeholder for nested 'query' object (copied from user's example)
    QJsonObject queryInner;
    queryInner["sql"] = "username = '" + username + "'";
    QJsonObject queryOuter;
    queryOuter["request_id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    queryOuter["action"] = "fetch_by_query";
    queryOuter["entity"] = "ids";
    queryOuter["query"] = queryInner;

    rootObject["query"] = queryOuter;
    rootObject["data"] = dataObject; // Single primary record
    rootObject["extracted_data"] = extractedItemsData; // Bulk records for 'items'
    rootObject["categories_data"] = extractedCategoriesData; // Bulk records for 'categories'

    qInfo() << "Extraction and JSON generation complete.";
    // Example: Print the resulting JSON document
    // qInfo() << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);

    return rootObject;
}
