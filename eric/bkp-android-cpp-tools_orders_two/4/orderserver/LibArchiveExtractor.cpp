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

// Helper to read content of a specific file inside an open archive
QByteArray LibArchiveExtractor::readArchiveEntryContent(struct archive *a, struct archive_entry *entry) {
    QByteArray content;
    if (archive_entry_size(entry) > 0) {
        content.resize(archive_entry_size(entry));
        la_ssize_t bytes_read = archive_read_data(a, content.data(), content.size());
        if (bytes_read < content.size()) {
            qWarning() << "readArchiveEntryContent: Warning: Only read" << bytes_read << "bytes out of expected" << content.size();
            content.resize(bytes_read);
        }
    } else {
        // If size is zero or unknown, read in chunks
        char buffer[10240];
        la_ssize_t len;
        while ((len = archive_read_data(a, buffer, sizeof(buffer))) > 0) {
            content.append(buffer, len);
        }
        if (len < 0) {
            qWarning() << "readArchiveEntryContent: Error reading data:" << archive_error_string(a);
            content.clear();
        }
    }
    return content;
}

// --- Core Logic: Pass 1 (Metadata Finding) ---

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

    int r = archive_read_open_fd(a, archiveFile.handle(), 10240);
    if (r != ARCHIVE_OK) {
        qWarning() << "findMetadataInArchive: archive_read_open_fd failed:" << archive_error_string(a);
        archive_read_free(a);
        archiveFile.close();
        return metadata;
    }

    struct archive_entry *entry;
    // Flags for required files
    bool foundUsername = false;
    bool foundCategory = false;
    bool foundItems = false;
    bool foundPrice = false;
    bool foundSizeOrQuantity = false;
    bool foundManufacturer = false;
    bool foundDescription = false;
    // Flags for optional/defaulted files
    bool foundStandardized = false;
    bool foundStandardizedAvatar = false;


    // First pass: Iterate through headers to find metadata files
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        QString entryName = QString::fromUtf8(archive_entry_pathname(entry));
        QString normalizedName = entryName.toLower().trimmed().replace('\\', '/');
        if (normalizedName.contains('/')) {
            normalizedName = normalizedName.mid(normalizedName.lastIndexOf('/') + 1);
        }

        // Helper lambda for reading and splitting content
        auto readAndSplit = [&](QStringList &list, bool &foundFlag, const QString &fileToCheck) {
            if (normalizedName == fileToCheck && !foundFlag) {
                QByteArray content = readArchiveEntryContent(a, entry);
                QString contentStr = QString::fromUtf8(content);
                // Split using QRegularExpression to handle \r\n, \r, or \n as delimiters, skipping empty parts
                list = contentStr.split(QRegularExpression("[\\r\\n]+"), Qt::SkipEmptyParts);
                qInfo() << "Metadata found:" << fileToCheck << "count=" << list.count();
                foundFlag = true;
                return true;
            }
            return false;
        };

        // Call the helper for all files
        bool isMetadata = false;
        isMetadata |= readAndSplit(metadata.usernames, foundUsername, "username.txt");
        isMetadata |= readAndSplit(metadata.categories, foundCategory, "category.txt");
        isMetadata |= readAndSplit(metadata.items, foundItems, "items.txt");
        isMetadata |= readAndSplit(metadata.prices, foundPrice, "price.txt");
        isMetadata |= readAndSplit(metadata.sizesOrQuantities, foundSizeOrQuantity, "sizeorquantity.txt");
        isMetadata |= readAndSplit(metadata.manufacturers, foundManufacturer, "manufacturer.txt");
        isMetadata |= readAndSplit(metadata.descriptions, foundDescription, "description.txt");
        isMetadata |= readAndSplit(metadata.standardized, foundStandardized, "standardized.txt");
        isMetadata |= readAndSplit(metadata.standardizedAvatars, foundStandardizedAvatar, "standardizedavatar.txt");


        // If we found all *required* files, we can break early.
        if (foundUsername && foundCategory && foundItems && foundPrice && foundSizeOrQuantity && foundManufacturer && foundDescription) {
            break;
        }

        // If the entry wasn't one of our metadata files, skip its data
        if (!isMetadata) {
            archive_read_data_skip(a);
        }
    }

    // Clean up first pass resources
    archive_read_close(a);
    archive_read_free(a);
    archiveFile.close();

    return metadata;
}


// --- Core Logic: Pass 2 (Extraction and JSON Generation) ---

QJsonObject LibArchiveExtractor::extractAndProcessTarXzFile(const QString &sourceFilePath,
                                                            const QString &destinationDir,
                                                            const QString &baseName) {

    // 1. FIRST PASS: Find Metadata
    ArchiveMetadata metadata = findMetadataInArchive(sourceFilePath);

    // Initial consistency checks (check only the required fields)
    if (metadata.usernames.isEmpty() || metadata.categories.isEmpty() || metadata.items.isEmpty() ||
        metadata.prices.isEmpty() || metadata.sizesOrQuantities.isEmpty() ||
        metadata.manufacturers.isEmpty() || metadata.descriptions.isEmpty()) {

        qWarning() << "Extraction failed: One or more required metadata files are missing or empty.";
        return QJsonObject();
    }

    // Assign lists for clarity
    const QStringList usernamesList = metadata.usernames;
    const QStringList categoriesList = metadata.categories;
    const QStringList itemsList = metadata.items;
    const QStringList pricesList = metadata.prices;
    const QStringList sizesOrQuantitiesList = metadata.sizesOrQuantities;
    const QStringList manufacturersList = metadata.manufacturers;
    const QStringList descriptionsList = metadata.descriptions;

    // Use metadata lists or default to "later" if optional files are missing
    const QStringList standardizedList = metadata.standardized.isEmpty() ? QStringList(itemsList.count(), "later") : metadata.standardized;
    const QStringList standardizedAvatarsList = metadata.standardizedAvatars.isEmpty() ? QStringList(itemsList.count(), "later") : metadata.standardizedAvatars;


    // Determine primary (first) values for the main 'data' object
    const QString primaryUsername = usernamesList.first();
    const QString primaryCategory = categoriesList.first();
    const QString primaryItemName = itemsList.first();
    const QString primaryPrice = pricesList.first();
    const QString primarySizeOrQuantity = sizesOrQuantitiesList.first();
    const QString primaryManufacturer = manufacturersList.first();
    const QString primaryDescription = descriptionsList.first();
    const QString primaryStandardized = standardizedList.first();
    const QString primaryStandardizedAvatar = standardizedAvatarsList.first();


    // 2. SECOND PASS: Extraction setup

    QFile archiveFile(sourceFilePath);
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

    // Primary IDs and timestamp
    QUuid primaryIdsId = QUuid::createUuid();
    QString createdAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // Primary 'data' object
    QJsonObject dataObject;
    dataObject["items_id"] = QUuid::createUuid().toString(QUuid::WithoutBraces); // Placeholder
    dataObject["ids_id"] = primaryIdsId.toString(QUuid::WithoutBraces);
    dataObject["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    dataObject["username"] = primaryUsername;
    dataObject["name"] = primaryItemName;
    dataObject["category"] = primaryCategory;
    dataObject["mediafile"] = ""; // Set later
    dataObject["sizeorquantity"] = primarySizeOrQuantity;
    dataObject["standardized"] = primaryStandardized;
    dataObject["standardizedavatar"] = primaryStandardizedAvatar;
    dataObject["manufacturer"] = primaryManufacturer;
    dataObject["price"] = primaryPrice;
    dataObject["description"] = primaryDescription;
    dataObject["created_at"] = createdAt;

    struct archive_entry *entry;
    int mediaFileIndex = 0; // Tracks the current media file being processed (0-indexed)

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

        // Skip all metadata TXT files in the second pass
        if (normalizedName == "username.txt" || normalizedName == "category.txt" ||
            normalizedName == "items.txt" || normalizedName == "price.txt" ||
            normalizedName == "sizeorquantity.txt" || normalizedName == "manufacturer.txt" ||
            normalizedName == "description.txt" || normalizedName == "standardized.txt" ||
            normalizedName == "standardizedavatar.txt") {
            archive_read_data_skip(a);
            continue;
        }

        // Only process and extract regular files (media)
        if (archive_entry_filetype(entry) != AE_IFREG) {
            archive_read_data_skip(a);
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

        // Determine the indices using modulo operator
        auto getIndex = [&](int totalCount) {
            return mediaFileIndex % totalCount;
        };

        const int userIndex = getIndex(usernamesList.count());
        const int catIndex = getIndex(categoriesList.count());
        const int itemIndex = getIndex(itemsList.count());
        const int priceIndex = getIndex(pricesList.count());
        const int sizeOrQuantityIndex = getIndex(sizesOrQuantitiesList.count());
        const int manufacturerIndex = getIndex(manufacturersList.count());
        const int descriptionIndex = getIndex(descriptionsList.count());
        const int standardizedIndex = getIndex(standardizedList.count());
        const int standardizedAvatarIndex = getIndex(standardizedAvatarsList.count());

        // Fetch current indexed values
        const QString currentUsername = usernamesList.at(userIndex);
        const QString currentCategory = categoriesList.at(catIndex);
        const QString currentItemName = itemsList.at(itemIndex);
        const QString currentPrice = pricesList.at(priceIndex);
        const QString currentSizeOrQuantity = sizesOrQuantitiesList.at(sizeOrQuantityIndex);
        const QString currentManufacturer = manufacturersList.at(manufacturerIndex);
        const QString currentDescription = descriptionsList.at(descriptionIndex);
        const QString currentStandardized = standardizedList.at(standardizedIndex);
        const QString currentStandardizedAvatar = standardizedAvatarsList.at(standardizedAvatarIndex);


        // 1. Populate JSON arrays for "extracted_data" (items table)
        QUuid entryItemsId = QUuid::createUuid();
        QUuid entryUuid = QUuid::createUuid();

        itemsIdArray.append(entryItemsId.toString(QUuid::WithoutBraces));
        idsIdArray.append(primaryIdsId.toString(QUuid::WithoutBraces));
        uuidArray.append(entryUuid.toString(QUuid::WithoutBraces));
        mediaFileArray.append(newFileName);

        nameArray.append(currentItemName);
        categoryArray.append(currentCategory);
        usernameArray.append(currentUsername);

        sizeOrQuantityArray.append(currentSizeOrQuantity);
        standardizedArray.append(currentStandardized);
        standardizedAvatarArray.append(currentStandardizedAvatar);
        manufacturerArray.append(currentManufacturer);
        priceArray.append(currentPrice);
        descriptionArray.append(currentDescription);

        createdAtArray.append(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

        if (mediaFileIndex == 0) {
            // Set primary mediafile and items_id to the first extracted file's data
            dataObject["mediafile"] = newFileName;
            dataObject["items_id"] = entryItemsId.toString(QUuid::WithoutBraces);
        }

        // 2. Populate JSON arrays for "categories_data" (categories table)
        if (!addedCategories.contains(currentCategory)) {
            QUuid categoryUuid = QUuid::createUuid();

            categoryIdArray.append(categoryUuid.toString(QUuid::WithoutBraces));
            categoryNameArray.append(currentCategory);
            categoryUsernameArray.append(primaryUsername); // Use primary user for category entry
            categoryCreatedAtArray.append(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            categoryIdsIdArray.append(primaryIdsId.toString(QUuid::WithoutBraces));

            addedCategories.insert(currentCategory);
        }

        mediaFileIndex++; // Increment index for the next file
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
    queryInner["sql"] = "username = '" + primaryUsername + "'";
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
