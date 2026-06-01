#include "LibArchiveExtractor.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QUuid>
#include <QDateTime>
#include <QTextStream>
#include <QHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>

#include <archive.h>
#include <archive_entry.h>

// --- Existing basic extraction function implementations (Simplified for brevity) ---

bool LibArchiveExtractor::extractArchiveFromQrc(const QString &qrcFilePath, const QString &destinationDir) {
    QFile file(qrcFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open QRC archive:" << qrcFilePath;
        return false;
    }
    QByteArray archiveData = file.readAll();
    file.close();
    return extractTarXzFromMemory(archiveData.constData(), archiveData.size(), destinationDir);
}

bool LibArchiveExtractor::extractTarXzFromBuffer(const QByteArray &data, const QString &destinationDir) {
    return extractTarXzFromMemory(data.constData(), data.size(), destinationDir);
}

bool LibArchiveExtractor::extractTarXzFromMemory(const void* data, size_t size, const QString& destination) {
    // This function remains an archive extraction implementation, similar to the one in your original files.
    struct archive *a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_xz(a);

    struct archive *ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                                            ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);

    QDir destDir(destination);
    if (!destDir.mkpath(".")) {
        qWarning() << "Failed to create destination directory:" << destination;
        archive_read_free(a);
        archive_write_free(ext);
        return false;
    }

    struct archive_entry *entry;
    int r;

    r = archive_read_open_memory(a, data, size);
    if (r != ARCHIVE_OK) {
        qWarning() << "archive_read_open_memory() failed:" << archive_error_string(a);
        archive_read_free(a);
        archive_write_free(ext);
        return false;
    }

    r = ARCHIVE_OK;
    while (r == ARCHIVE_OK) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) break;
        if (r < ARCHIVE_OK) {
            qWarning() << "archive_read_next_header failed:" << archive_error_string(a);
            break;
        }

        QString entryPath = QString::fromUtf8(archive_entry_pathname(entry));
        QString fullOutputPath = destDir.filePath(entryPath);

        // Skip metadata files if they were passed via memory buffer (though unlikely)
        QString normalizedName = entryPath.toLower().trimmed().replace('\\', '/');
        if (normalizedName.contains('/')) {
            normalizedName = normalizedName.mid(normalizedName.lastIndexOf('/') + 1);
        }
        if (normalizedName == "username.txt" || normalizedName == "category.txt") {
            archive_read_data_skip(a);
            continue;
        }


        archive_entry_set_pathname(entry, fullOutputPath.toUtf8().constData());

        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            qWarning() << "archive_write_header failed:" << archive_error_string(ext)
            << " for entry:" << fullOutputPath;
        } else if (archive_entry_size(entry) > 0) {
            const void *buff;
            size_t blockSize;
            la_int64_t offset;

            while (true) {
                r = archive_read_data_block(a, &buff, &blockSize, &offset);
                if (r == ARCHIVE_EOF) break;
                if (r < ARCHIVE_OK) {
                    qWarning() << "archive_read_data_block error:" << archive_error_string(a) << " for entry:" << fullOutputPath;
                    break;
                }
                r = archive_write_data_block(ext, buff, blockSize, offset);
                if (r < ARCHIVE_OK) {
                    qWarning() << "archive_write_data_block error:" << archive_error_string(ext) << " for entry:" << fullOutputPath;
                    break;
                }
            }
        }
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
            qWarning() << "archive_write_finish_entry failed:" << archive_error_string(ext) << " after entry:" << fullOutputPath;
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    return true;
}

// --- Helper Function Implementations for TXT Reading (First Pass) ---

// Reads the content of a single entry from an *open* archive reader
QByteArray LibArchiveExtractor::readArchiveEntryContent(struct archive *a, struct archive_entry *entry) {
    QByteArray content;
    const void *buff;
    size_t blockSize;
    la_int64_t offset;
    int r;

    // Only read data for regular files
    if (archive_entry_filetype(entry) != AE_IFREG) {
        archive_read_data_skip(a);
        return content;
    }

    while (true) {
        r = archive_read_data_block(a, &buff, &blockSize, &offset);
        if (r == ARCHIVE_EOF) break;
        if (r < ARCHIVE_OK) {
            qWarning() << "Error reading data block for metadata file:" << archive_error_string(a);
            // Must skip data on error to continue iterating
            archive_read_data_skip(a);
            return QByteArray();
        }
        content.append(QByteArray(static_cast<const char*>(buff), static_cast<int>(blockSize)));
    }
    return content;
}

// Performs the first pass to find and extract metadata TXT files
ArchiveMetadata LibArchiveExtractor::findMetadataInArchive(const QString &sourceFilePath) {
    ArchiveMetadata metadata;

    struct archive *a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_xz(a);

    QFile archiveFile(sourceFilePath);
    if (!archiveFile.open(QIODevice::ReadOnly)) {
        qWarning() << "findMetadataInArchive: Failed to open archive file:" << sourceFilePath;
        archive_read_free(a);
        return metadata;
    }

    // Open file descriptor for reading
    // Use the file descriptor to allow libarchive to seek/read internally
    int r = archive_read_open_fd(a, archiveFile.handle(), 10240);
    if (r != ARCHIVE_OK) {
        qWarning() << "findMetadataInArchive: archive_read_open_fd failed:" << archive_error_string(a);
        archive_read_free(a);
        archiveFile.close();
        return metadata;
    }

    struct archive_entry *entry;
    bool foundUsername = false;
    bool foundCategory = false;

    // First pass: Iterate through headers to find metadata files
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        QString entryName = QString::fromUtf8(archive_entry_pathname(entry));

        // Normalize name: extract filename regardless of directory path (e.g., 'data/username.txt' -> 'username.txt')
        QString normalizedName = entryName.toLower().trimmed().replace('\\', '/');
        if (normalizedName.contains('/')) {
            normalizedName = normalizedName.mid(normalizedName.lastIndexOf('/') + 1);
        }

        if (normalizedName == "username.txt" && !foundUsername) {
            QByteArray content = readArchiveEntryContent(a, entry);
            // Read content, convert to string, and trim whitespace/newlines
            metadata.username = QString::fromUtf8(content).trimmed();
            qInfo() << "Metadata found: Username=" << metadata.username;
            foundUsername = true;
        }
        else if (normalizedName == "category.txt" && !foundCategory) {
            QByteArray content = readArchiveEntryContent(a, entry);
            // Read content, convert to string, and trim whitespace/newlines
            metadata.category = QString::fromUtf8(content).trimmed();
            qInfo() << "Metadata found: Category=" << metadata.category;
            foundCategory = true;
        } else {
            // Skip data for other files without reading
            archive_read_data_skip(a);
        }

        if (foundUsername && foundCategory) {
            break; // Found both, exit early
        }
    }

    // Clean up first pass resources
    archive_read_close(a);
    archive_read_free(a);
    // The QFile will be closed when the function returns/goes out of scope, but we close it manually here for clarity
    archiveFile.close();

    return metadata;
}


// --- Core Function Implementation (Using 2-pass logic) ---

QJsonObject LibArchiveExtractor::extractAndProcessTarXzFile(const QString &sourceFilePath,
                                                            const QString &destinationDir,
                                                            const QString &baseName) {

    // 1. FIRST PASS: Find Metadata (username, category)
    ArchiveMetadata metadata = findMetadataInArchive(sourceFilePath);

    if (metadata.username.isEmpty()) {
        qWarning() << "Extraction failed: Required metadata 'username.txt' not found or is empty in archive.";
        return QJsonObject();
    }
    if (metadata.category.isEmpty()) {
        qWarning() << "Extraction failed: Required metadata 'category.txt' not found or is empty in archive.";
        return QJsonObject();
    }

    const QString username = metadata.username;
    const QString category = metadata.category;

    // 2. SECOND PASS: Extraction and JSON Generation

    QFile archiveFile(sourceFilePath);
    // We must reopen the file for the second pass
    if (!archiveFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open archive file for second pass:" << sourceFilePath;
        return QJsonObject();
    }

    struct archive *a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_xz(a);

    struct archive *ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                                            ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);

    // Re-open file descriptor for reading
    int r = archive_read_open_fd(a, archiveFile.handle(), 10240);
    if (r != ARCHIVE_OK) {
        qWarning() << "Second pass: archive_read_open_fd failed:" << archive_error_string(a);
        archive_read_free(a);
        archive_write_free(ext);
        archiveFile.close();
        return QJsonObject();
    }

    QDir destination(destinationDir);
    if (!destination.mkpath(".")) {
        qWarning() << "Second pass: Failed to create destination directory:" << destinationDir;
        archive_read_close(a);
        archive_read_free(a);
        archive_write_free(ext);
        archiveFile.close();
        return QJsonObject();
    }

    // JSON array preparation
    QJsonArray itemsIdArray, idsIdArray, uuidArray, mediaFileArray, nameArray, categoryArray,
        usernameArray, sizeOrQuantityArray, standardizedArray, standardizedAvatarArray,
        manufacturerArray, priceArray, descriptionArray, createdAtArray;
    QJsonArray categoryIdArray, categoryNameArray, categoryUsernameArray, categoryCreatedAtArray, categoryIdsIdArray;
    QSet<QString> addedCategories;

    // Primary IDs
    QUuid primaryIdsId = QUuid::createUuid();
    QString createdAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // Primary 'data' object (single record for 'ids' table)
    QJsonObject dataObject;
    dataObject["items_id"] = QUuid::createUuid().toString(QUuid::WithoutBraces); // Placeholder
    dataObject["ids_id"] = primaryIdsId.toString(QUuid::WithoutBraces);
    dataObject["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    dataObject["username"] = username;
    dataObject["name"] = baseName;
    dataObject["category"] = category;
    dataObject["mediafile"] = "";
    dataObject["sizeorquantity"] = "N/A";
    dataObject["standardized"] = "later";
    dataObject["standardizedavatar"] = "later";
    dataObject["manufacturer"] = "later";
    dataObject["price"] = "N/A";
    dataObject["description"] = "Primary item record for archive: " + QFileInfo(sourceFilePath).fileName();
    dataObject["created_at"] = createdAt;

    struct archive_entry *entry;
    int mediaFileIndex = 0;

    // Second pass: Extract media files and generate JSON metadata
    r = ARCHIVE_OK;
    while (r == ARCHIVE_OK) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) break;
        if (r < ARCHIVE_OK) {
            qWarning() << "Second pass: Error reading next header:" << archive_error_string(a);
            break;
        }

        QString originalFileName = QString::fromUtf8(archive_entry_pathname(entry));
        QString normalizedName = originalFileName.toLower().trimmed().replace('\\', '/');
        if (normalizedName.contains('/')) {
            normalizedName = normalizedName.mid(normalizedName.lastIndexOf('/') + 1);
        }

        // Skip the metadata TXT files in the second pass
        if (normalizedName == "username.txt" || normalizedName == "category.txt") {
            archive_read_data_skip(a);
            // No need to call archive_write_finish_entry(ext) here as we never called archive_write_header(ext, entry)
            continue;
        }

        // Only process and extract regular files (media)
        if (archive_entry_filetype(entry) != AE_IFREG) {
            archive_read_data_skip(a);
            // No need to call archive_write_finish_entry(ext) here
            continue;
        }

        QFileInfo originalFileInfo(originalFileName);

        // Generate new file name (UUID + original extension)
        QUuid fileUuid = QUuid::createUuid();
        QString newFileName = fileUuid.toString(QUuid::WithoutBraces) + "." + originalFileInfo.suffix();
        QString fullPath = destination.filePath(newFileName);

        // Update entry pathname to the full path before writing
        archive_entry_set_pathname(entry, fullPath.toUtf8().constData());

        // Write header and data block by block
        int write_r = archive_write_header(ext, entry);
        if (write_r != ARCHIVE_OK) {
            qWarning() << "Second pass: archive_write_header failed:" << archive_error_string(ext)
            << " for entry:" << fullPath;
        } else if (archive_entry_size(entry) > 0) {
            const void *buff;
            size_t blockSize;
            la_int64_t offset;

            while (true) {
                int read_r = archive_read_data_block(a, &buff, &blockSize, &offset);
                if (read_r == ARCHIVE_EOF) break;
                if (read_r < ARCHIVE_OK) {
                    qWarning() << "Second pass: archive_read_data_block error:" << archive_error_string(a)
                    << " for entry:" << fullPath;
                    break;
                }

                write_r = archive_write_data_block(ext, buff, blockSize, offset);
                if (write_r < ARCHIVE_OK) {
                    qWarning() << "Second pass: archive_write_data_block error:" << archive_error_string(ext)
                    << " for entry:" << fullPath;
                    break;
                }
            }
        }

        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
            qWarning() << "Second pass: archive_write_finish_entry failed:" << archive_error_string(ext)
            << " after entry:" << fullPath;
        }

        // --- JSON Data Population ---

        // 1. Populate JSON arrays for "extracted_data" (items table)
        QUuid entryItemsId = QUuid::createUuid();
        QUuid entryUuid = QUuid::createUuid();

        itemsIdArray.append(entryItemsId.toString(QUuid::WithoutBraces));
        idsIdArray.append(primaryIdsId.toString(QUuid::WithoutBraces));
        uuidArray.append(entryUuid.toString(QUuid::WithoutBraces));
        mediaFileArray.append(newFileName);

        QString entryName = baseName + QString::number(++mediaFileIndex);
        nameArray.append(entryName);
        categoryArray.append(category);
        usernameArray.append(username);
        sizeOrQuantityArray.append("N/A");
        standardizedArray.append("later");
        standardizedAvatarArray.append("later");
        manufacturerArray.append("later");
        priceArray.append("N/A");
        descriptionArray.append("File extracted: " + originalFileName + " by " + username);
        createdAtArray.append(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

        if (mediaFileIndex == 1) {
            // Set primary mediafile and items_id to the first extracted file's data
            dataObject["mediafile"] = newFileName;
            dataObject["items_id"] = entryItemsId.toString(QUuid::WithoutBraces);
        }

        // 2. Populate JSON arrays for "categories_data" (categories table)
        if (!addedCategories.contains(category)) {
            QUuid categoryUuid = QUuid::createUuid();

            categoryIdArray.append(categoryUuid.toString(QUuid::WithoutBraces));
            categoryNameArray.append(category);
            categoryUsernameArray.append(username);
            categoryCreatedAtArray.append(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            categoryIdsIdArray.append(primaryIdsId.toString(QUuid::WithoutBraces));

            addedCategories.insert(category);
        }
    }

    // Clean up second pass
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    archiveFile.close();

    // 3. Assemble final JSON object
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

    QJsonObject extractedCategoriesData;
    extractedCategoriesData["categories_id"] = categoryIdArray;
    extractedCategoriesData["name"] = categoryNameArray;
    extractedCategoriesData["username"] = categoryUsernameArray;
    extractedCategoriesData["created_at"] = categoryCreatedAtArray;
    extractedCategoriesData["ids_id"] = categoryIdsIdArray;

    QJsonObject rootObject;
    rootObject["request_id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    rootObject["action"] = "insert";
    rootObject["entity"] = "items";

    // Placeholder for nested 'query' object
    QJsonObject queryInner;
    queryInner["sql"] = "username = '" + username + "'";
    QJsonObject queryOuter;
    queryOuter["request_id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    queryOuter["action"] = "fetch_by_query";
    queryOuter["entity"] = "ids";
    queryOuter["query"] = queryInner;

    rootObject["query"] = queryOuter;
    rootObject["data"] = dataObject;
    rootObject["extracted_data"] = extractedItemsData;
    rootObject["categories_data"] = extractedCategoriesData;

    qInfo() << "Extraction and JSON generation complete. Processed" << mediaFileIndex << "media files.";

    return rootObject;
}
