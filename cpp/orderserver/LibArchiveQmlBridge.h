// LibArchiveQmlBridge.h
#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject>

class LibArchiveQmlBridge : public QObject
{
    Q_OBJECT
public:
    explicit LibArchiveQmlBridge(QObject *parent = nullptr);
    ~LibArchiveQmlBridge() override = default;

    // Singleton
    static LibArchiveQmlBridge *instance;
    static LibArchiveQmlBridge *getInstance();

    // Existing extraction methods
    Q_INVOKABLE bool extractTarXzFromQrc(const QString &qrcResourcePath, const QString &destinationDir);
    Q_INVOKABLE bool extractTarXzFromBuffer(const QByteArray &data, const QString &destinationDir);
    Q_INVOKABLE bool extractTarXzFromMemory(const QVariant &bufferVariant, const QString &destinationDir);
    Q_INVOKABLE bool extractTarXzFromMemoryVariant(const QVariant &bufferVariant, const QString &destinationDir);

    /**
     * @brief Core QML function to process an archive.
     * * Reads metadata (username, category) from the archive itself, extracts media,
     * and generates the JSON payload.
     * * @param fileUrl The QML file path (e.g., "file:///...") to the archive.
     * @param baseName The base name for the archive data entry.
     * @return QJsonObject containing the metadata result.
     */
    Q_INVOKABLE QJsonObject processExtractedArchive(const QString &fileUrl,
                                                    const QString &baseName);

    // Compression (calls LibArchiveCompressor - Placeholder for actual implementation)
    Q_INVOKABLE bool compressFolder(const QString &folderPath, const QString &outputDir = QString(), const QString &archiveName = QString());
};
