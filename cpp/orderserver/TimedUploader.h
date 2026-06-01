#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QHash>
#include <QSet>
#include <QTimer>
#include <QStringList> // Required for ordered processing

class DataUploader;
class DataNet;

class TimedUploader : public QObject
{
    Q_OBJECT

public:
    explicit TimedUploader(QObject *parent = nullptr, DataUploader *uploader = nullptr, DataNet *mydatanet = nullptr);

    /**
     * @brief Initiates the asynchronous, two-phase, timed upload process.
     * @param payload The QJsonObject containing "categories_data" and "extracted_data".
     */
    Q_INVOKABLE void uploadExtractedItems(const QJsonObject& payload);

private Q_SLOTS:
    /**
     * @brief Slot connected to the timer. Processes one category or one item transaction per call.
     */
    void processNextItem();

private:
    // New: Enum to track the upload phase
    enum UploadPhase {
        CategoryPhase, // Process all categories first
        ItemPhase,     // Process all items second
        Finished
    };
    UploadPhase m_currentPhase = Finished;

    // Dependencies (Retained from your code)
    DataUploader *uploader_;
    DataNet *mydatanet_;

    // --- DataNet API Dispatch Methods (Implemented in .cpp) ---
    void newcategory(QVariantMap info);
    void newitem(QVariantMap info);
    void updateitem(QVariantMap info);

    // --- State Variables for Asynchronous Upload ---
    QTimer m_uploadTimer;
    QTimer m_startitemdelayTimer;

    // Index will be reused for category iteration and item iteration
    int m_currentIndex = 0;
    int m_itemCount = 0;
    int m_categoryCount = 0;

    QJsonObject m_extractedData;
    QHash<QString, QVariantMap> m_categoryLookup;

    // Ordered list of unique categories required by "new" items
    QStringList m_categoryNamesToUpload;

    // Counters for final log
    int m_newCount = 0;
    int m_updateCount = 0;
    int m_categoriesUploaded = 0;
    int m_skippedCount = 0;

    // Helper functions for phase logic
    void buildCategoryLookup(const QJsonObject& payload);
    void processCategoryPhase();
    void processItemPhase();
    void startafterdelay();
};
