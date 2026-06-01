#include "DataUploader.h"
#include <QDebug>
#include <QSet>
#include <QHash>
#include "base/DataNet.h"
#include "base/MapperSoftware.h"


// --- DataUploader Implementation ---

DataUploader::DataUploader(QObject *parent,DataNet* dataNetInstance,MapperSoftware *mappersoftware,Wt::Http::Client *client,std::shared_ptr<MyJsonTools> mypayload_,std::shared_ptr<eric::JsonUtils> myjsonutils_)
    : QObject(parent), m_dataNet(dataNetInstance), mappersoftware_(mappersoftware),client_(client),mypayload(mypayload_),myjsonutils(myjsonutils_) // Initialize the mDataNet member with the provided pointer
{

    if(!m_dataNet) {
    // Constructor body: Add any additional setup logic here
    if (dataNetInstance) {
        qInfo() << "DataUploader initialized with a valid DataNet instance.";
        // Connect signals/slots, etc.
    }

    else if(!(dataNetInstance) && (mappersoftware && client && mypayload_ && myjsonutils_)) {

        m_dataNet =std::make_shared<DataNet>(mappersoftware,client,mypayload_,myjsonutils_);
        client_->done().connect(m_dataNet.get(), &DataNet::requestDone);

      }

    else {
        qInfo() << "DataUploader initialized without a DataNet instance; network operations may be limited.";
        DataNet *mynet = DataNet::getInstance();
        m_dataNet = std::shared_ptr<DataNet>(mynet);
    }

    }

    else {

        qInfo() << "DataNet is Valid" << Qt::endl;
    }
}

// --- Placeholder DataNet API Call Methods ---
// NOTE: You must replace these placeholders with the actual calls to your DataNet instance
void DataUploader::newcategory(QVariantMap info) {
    qInfo() << "API Call: newcategory for:" << info.value("name").toString() << "ID:" << info.value("categories_id").toString();
    m_dataNet->newcategory(info);
}
void DataUploader::newitem(QVariantMap info) {
    qInfo() << "API Call: newitem for:" << info.value("name").toString() << "ActionDue: New";
    m_dataNet->newitem(info);
}
void DataUploader::updateitem(QVariantMap info) {
    qInfo() << "API Call: updateitem for:" << info.value("name").toString() << "ActionDue: Update";
    m_dataNet->updateitem(info);
}


// --- Core Iteration and Dispatch Logic ---

void DataUploader::uploadExtractedItems(const QJsonObject& payload)
{
    if (payload.isEmpty()) {
        qWarning() << "uploadExtractedItems: Input payload is empty.";
        return;
    }

    // --- 1. Prepare Category Lookup Map ---
    // Maps category name -> {categories_id, user, name, created_at, ids_id}
    QHash<QString, QVariantMap> categoryLookup;
    QJsonObject categoryData = payload["categories_data"].toObject();
    
    QJsonArray catNamesArray = categoryData["name"].toArray();
    
    int catCount = catNamesArray.size();
    for (int j = 0; j < catCount; ++j) {
        QVariantMap categoryDetails;
        QString name = catNamesArray.at(j).toString();
        
        // Map all necessary category data fields using the category index (j)
        categoryDetails["name"] = name; // CRITICAL: The category name
        categoryDetails["categories_id"] = categoryData["categories_id"].toArray().at(j).toString();
        categoryDetails["user"] = categoryData["username"].toArray().at(j).toString(); 
        categoryDetails["ids_id"] = categoryData["ids_id"].toArray().at(j).toString();
        categoryDetails["created_at"] = categoryData["created_at"].toArray().at(j).toString();
        
        categoryLookup.insert(name, categoryDetails);
    }
    
    qInfo() << "Category lookup map built with" << catCount << "unique categories.";

    // --- 2. Process Items (Single-Pass with conditional logic) ---
    qInfo() << "Starting item and conditional category upload...";
    QJsonObject extractedData = payload["extracted_data"].toObject();
    
    // Get the arrays needed for iteration and condition checks
    QJsonArray actionDueArray = extractedData["actiondue"].toArray();
    QJsonArray itemNames = extractedData["name"].toArray();
    int itemCount = itemNames.size();
    
    // Set to track categories that have already been successfully uploaded in this batch
    QSet<QString> uploadedCategories;
    
    int newCount = 0;
    int updateCount = 0;
    int categoriesUploaded = 0;
    int skippedCount = 0;

    for (int i = 0; i < itemCount; ++i) {
        
        // --- A. Extract common item fields using item index (i) ---
        QVariantMap itemInfo;
        
        const QString currentUsername = extractedData["username"].toArray().at(i).toString();
        const QString currentCategory = extractedData["category"].toArray().at(i).toString();
        const QString currentActionDue = actionDueArray.at(i).toString().toLower().trimmed();
        
        // Map ALL fields required by newitem/updateitem
        itemInfo["itemsid"] = extractedData["items_id"].toArray().at(i).toString();
        itemInfo["user"] = currentUsername;
        itemInfo["name"] = itemNames.at(i).toString();
        itemInfo["category"] = currentCategory;
        itemInfo["ids_id"] = extractedData["ids_id"].toArray().at(i).toString();
        itemInfo["mediafile"] = extractedData["mediafile"].toArray().at(i).toString();
        itemInfo["price"] = extractedData["price"].toArray().at(i).toString();
        itemInfo["description"] = extractedData["description"].toArray().at(i).toString();
        itemInfo["sizeorquantity"] = extractedData["sizeorquantity"].toArray().at(i).toString();
        itemInfo["uuid"] = extractedData["uuid"].toArray().at(i).toString();
        itemInfo["standardized"] = extractedData["standardized"].toArray().at(i).toString();
        itemInfo["standardizedavatar"] = extractedData["standardizedavatar"].toArray().at(i).toString();
        itemInfo["manufacturer"] = extractedData["manufacturer"].toArray().at(i).toString();
        itemInfo["created_at"] = extractedData["created_at"].toArray().at(i).toString(); 

        
        // --- B. Conditional API Call ---
        
        if (currentActionDue == "new") {
            
            // 1. Check if the category needs to be uploaded first
            if (!uploadedCategories.contains(currentCategory)) {
                
                if (categoryLookup.contains(currentCategory)) {
                    QVariantMap categoryInfo = categoryLookup.value(currentCategory);
                    
                    // Call API to create the category with all extracted metadata
                    newcategory(categoryInfo); // Placeholder call
                    
                    // Assume success and mark as uploaded
                    uploadedCategories.insert(currentCategory);
                    categoriesUploaded++;
                } else {
                    qWarning() << "Error: Category" << currentCategory << "not found in categories_data for new item at index" << i;
                    // Decide whether to skip item or continue with item creation (continuing may cause foreign key failure)
                }
            }
            
            // 2. Upload the New Item
            newitem(itemInfo); // Placeholder call
            newCount++;
            
        } else if (currentActionDue == "update") {
            
            // Update Item Logic (no category check needed)
            updateitem(itemInfo); // Placeholder call
            updateCount++;
            
        } else {
            qWarning() << "Skipping item at index" << i << ": Unknown actiondue value:" << currentActionDue;
            skippedCount++;
        }
    }
    
    qInfo() << "Upload session finished. Items processed:" << itemCount << ". (New:" << newCount << ", Update:" << updateCount << ", Skipped:" << skippedCount << "). Categories uploaded:" << categoriesUploaded;
}
