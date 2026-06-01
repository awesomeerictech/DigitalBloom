#include "TimedUploader.h"
#include <QDebug>
#include <QSet>
#include <QHash>
#include <QJsonArray>
#include "base/DataNet.h"
#include "DataUploader.h"


// --- Constructor and Initialization (Retained) ---

TimedUploader::TimedUploader(QObject *parent, DataUploader *uploader, DataNet *mydatanet)
    : QObject(parent),uploader_(uploader), mydatanet_(mydatanet)
{
    // Connect the timer to the slot and set the delay
    QObject::connect(
        &m_uploadTimer,
        &QTimer::timeout,
        this,
        &TimedUploader::processNextItem
        );

    m_uploadTimer.setInterval(100); // 1000 ms = 1 second delay between calls
}

// --- DataNet API Dispatch Methods (Retained - Assumes these dispatch to the network) ---

void TimedUploader::newcategory(QVariantMap info) {
    qInfo() << "API Call: newcategory for:" << info.value("name").toString() << "ID:" << info.value("categories_id").toString();
    if(uploader_) {
        uploader_->newcategory(info);
    }

    else if(mydatanet_) {

        mydatanet_->newcategory(info);
    }

    else {

        qWarning() << "Not API found to call" << Qt::endl;

    }
}

void TimedUploader::newitem(QVariantMap info) {
    qInfo() << "API Call: newitem for:" << info.value("name").toString() << "ActionDue: New";

    if(uploader_) {

        uploader_->newitem(info);
    }

    else if(mydatanet_) {

        mydatanet_->newitem(info);
    }

    else {

        qWarning() << "Not API found to call" << Qt::endl;

    }
}

void TimedUploader::updateitem(QVariantMap info) {
    qInfo() << "API Call: updateitem for:" << info.value("name").toString() << "ActionDue: Update";

    if(uploader_) {
        uploader_->updateitem(info);
    }

    else if(mydatanet_) {

        mydatanet_->updateitem(info);
    }

    else {

        qWarning() << "Not API found to call" << Qt::endl;

    }
}


// --- Helper to build category lookup map (Retained) ---
void TimedUploader::buildCategoryLookup(const QJsonObject& payload)
{
    m_categoryLookup.clear();
    QJsonObject categoryData = payload["categories_data"].toObject();

    QJsonArray catNamesArray = categoryData["name"].toArray();
    QJsonArray catIdsArray = categoryData["categories_id"].toArray();
    QJsonArray catUsernamesArray = categoryData["username"].toArray();
    QJsonArray catCreatedArray = categoryData["created_at"].toArray();
    QJsonArray catIdsIdsArray = categoryData["ids_id"].toArray();

    int catCount = catNamesArray.size();
    for (int j = 0; j < catCount; ++j) {
        QVariantMap categoryDetails;
        QString name = catNamesArray.at(j).toString();

        categoryDetails["name"] = name;
        categoryDetails["categories_id"] = catIdsArray.at(j).toString();
        categoryDetails["user"] = catUsernamesArray.at(j).toString();
        categoryDetails["ids_id"] = catIdsIdsArray.at(j).toString();
        categoryDetails["created_at"] = catCreatedArray.at(j).toString();

        m_categoryLookup.insert(name, categoryDetails);
    }

    qInfo() << "Category lookup map built with" << catCount << "unique categories.";
}


// --- Initiator Function (Restructured to initialize the two phases) ---

void TimedUploader::uploadExtractedItems(const QJsonObject& payload)
{
    if (payload.isEmpty() || !payload.contains("extracted_data")) {
        qWarning() << "Upload failed: Invalid or empty payload.";
        return;
    }

    // 1. Reset State and Process Initial Data
    m_currentIndex = 0;
    m_newCount = 0;
    m_updateCount = 0;
    m_categoriesUploaded = 0;
    m_skippedCount = 0;
    m_categoryNamesToUpload.clear();
    m_currentPhase = Finished; // Default to finished

    m_extractedData = payload["extracted_data"].toObject();
    m_itemCount = m_extractedData["name"].toArray().size();

    // Build the full category lookup map
    buildCategoryLookup(payload);

    // 2. Determine which categories need to be created (needed by "new" items)
    QSet<QString> uniqueCategoriesNeeded;
    QJsonArray actionDueArray = m_extractedData["actiondue"].toArray();
    QJsonArray categoryArray = m_extractedData["category"].toArray();

    for (int i = 0; i < m_itemCount; ++i) {
        if (actionDueArray.at(i).toString().toLower().trimmed() == "new") {
            uniqueCategoriesNeeded.insert(categoryArray.at(i).toString());
        }
    }

    // Populate the ordered list of categories to upload
    for (const QString& categoryName : uniqueCategoriesNeeded) {
        if (m_categoryLookup.contains(categoryName)) {
            m_categoryNamesToUpload.append(categoryName);
        } else {
            qWarning() << "Warning: New item requires category" << categoryName << "but category metadata is missing in payload.";
        }
    }

    m_categoryCount = m_categoryNamesToUpload.size();

    // 3. Set starting phase
    if (m_categoryCount > 0) {
        m_currentPhase = CategoryPhase;
        qInfo() << "Starting timed upload of" << m_categoryCount << "categories (Phase 1) and" << m_itemCount << "items (Phase 2).";
    } else if (m_itemCount > 0) {
        m_currentPhase = ItemPhase;
        qInfo() << "No categories to upload. Skipping to Item Phase (1000ms delay for items).";
    } else {
        qInfo() << "No items or categories to upload.";
        return;
    }

    // Run the first transaction immediately to start the sequence
    processNextItem();
}


// --- Timed Slot (State Machine Logic) ---

void TimedUploader::processNextItem()
{
    m_uploadTimer.stop();

    if (m_currentPhase == CategoryPhase) {
        if (m_currentIndex < m_categoryCount) {
            // Process current category, then schedule the next one
            processCategoryPhase();
            m_currentIndex++;

            if (m_currentIndex < m_categoryCount) {
                m_uploadTimer.start(); // Set timer for next category
            } else {
                // Category phase complete. Reset index and move to Item Phase.
                m_currentIndex = 0;
                m_currentPhase = ItemPhase;
                qInfo() << "Category upload phase complete. Starting Item upload phase...";

                // Start item phase immediately if there are items
                if (m_itemCount > 0) {
                    processNextItem(); // Start the Item Phase without delay
                } else {
                    processNextItem(); // Go to termination log
                }
            }
        }
    }

    else if (m_currentPhase == ItemPhase) {
        if (m_currentIndex < m_itemCount) {
            // Process current item, then schedule the next one
            processItemPhase();
            m_currentIndex++;

            if (m_currentIndex < m_itemCount) {
                m_uploadTimer.start(); // Set timer for next item
            } else {
                // Item phase complete. Move to Finished Phase.
                m_currentPhase = Finished;
                processNextItem(); // Go to termination log
            }
        }
    }

    else if (m_currentPhase == Finished) {
        qInfo() << "Upload session finished. Total processed: Categories:" << m_categoriesUploaded
                << ", Items (New:" << m_newCount << ", Update:" << m_updateCount
                << ", Skipped:" << m_skippedCount << ").";
        return;
    }
}


// --- Specific Phase Implementations ---

void TimedUploader::processCategoryPhase() {
    const QString categoryName = m_categoryNamesToUpload.at(m_currentIndex);

    // Retrieve the pre-built map data
    QVariantMap categoryInfo = m_categoryLookup.value(categoryName);

    // Call API to create the category
    newcategory(categoryInfo);
    m_categoriesUploaded++;
}

void TimedUploader::processItemPhase() {
    const int i = m_currentIndex;

    QJsonArray actionDueArray = m_extractedData["actiondue"].toArray();
    QJsonArray itemNames = m_extractedData["name"].toArray();

    QVariantMap itemInfo;
    const QString currentUsername = m_extractedData["username"].toArray().at(i).toString();
    const QString currentCategory = m_extractedData["category"].toArray().at(i).toString();
    const QString currentActionDue = actionDueArray.at(i).toString().toLower().trimmed();

    // Map ALL item fields (Retained)
    itemInfo["itemsid"] = m_extractedData["items_id"].toArray().at(i).toString();
    itemInfo["user"] = currentUsername;
    itemInfo["name"] = itemNames.at(i).toString();
    itemInfo["category"] = currentCategory;
    itemInfo["ids_id"] = m_extractedData["ids_id"].toArray().at(i).toString();
    itemInfo["mediafile"] = m_extractedData["mediafile"].toArray().at(i).toString();
    itemInfo["price"] = m_extractedData["price"].toArray().at(i).toString();
    itemInfo["description"] = m_extractedData["description"].toArray().at(i).toString();
    itemInfo["sizeorquantity"] = m_extractedData["sizeorquantity"].toArray().at(i).toString();
    itemInfo["uuid"] = m_extractedData["uuid"].toArray().at(i).toString();
    itemInfo["standardized"] = m_extractedData["standardized"].toArray().at(i).toString();
    itemInfo["standardizedavatar"] = m_extractedData["standardizedavatar"].toArray().at(i).toString();
    itemInfo["manufacturer"] = m_extractedData["manufacturer"].toArray().at(i).toString();
    itemInfo["created_at"] = m_extractedData["created_at"].toArray().at(i).toString();


    // Conditional Call based on actiondue
    if (currentActionDue == "new") {
        // Now it's safe to call newitem, as the category is already guaranteed to exist
        newitem(itemInfo);
        m_newCount++;

    } else if (currentActionDue == "update") {

        updateitem(itemInfo);
        m_updateCount++;

    } else {
        qWarning() << "Skipping item at index" << i << ": Unknown actiondue value:" << currentActionDue;
        m_skippedCount++;
    }
}
