// LibArchiveQmlBridge.h
#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject> // Ensure QJsonObject is included

class LibArchiveQmlBridge : public QObject
{
    Q_OBJECT
public:
    explicit LibArchiveQmlBridge(QObject *parent = nullptr);
    ~LibArchiveQmlBridge() override = default;

    // Extraction methods forwarded to your existing class (kept intact).
    Q_INVOKABLE bool extractTarXzFromQrc(const QString &qrcResourcePath, const QString &destinationDir);
    Q_INVOKABLE bool extractTarXzFromBuffer(const QByteArray &data, const QString &destinationDir);
    Q_INVOKABLE bool extractTarXzFromMemory(const QVariant &bufferVariant, const QString &destinationDir);
    Q_INVOKABLE bool extractTarXzFromMemoryVariant(const QVariant &bufferVariant, const QString &destinationDir);

    // New function for QML: opens file, extracts to standard location, and returns the JSON result
    Q_INVOKABLE QJsonObject processExtractedArchive(const QString &fileUrl,
                                                    const QString &username,
                                                    const QString &baseName,
                                                    const QString &category);

    // Compression (calls LibArchiveCompressor)
    Q_INVOKABLE bool compressFolder(const QString &folderPath, const QString &outputDir = QString(), const QString &archiveName = QString());
    // Singleton
    static LibArchiveQmlBridge *instance;
    static LibArchiveQmlBridge *getInstance();
};
