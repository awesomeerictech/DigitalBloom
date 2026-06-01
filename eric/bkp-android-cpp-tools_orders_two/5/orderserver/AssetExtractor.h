#pragma once

#include <QObject>
#include <QString>

class AssetExtractor : public QObject {
    Q_OBJECT

public:
    explicit AssetExtractor(QObject *parent = nullptr);

    // Extracts a .tar.xz file from a qrc path (e.g., ":/serverassets/AppArchive") into a directory
    Q_INVOKABLE bool extractLzmaArchive(const QString &qrcPath, const QString &outputDir);

    // Creates a .tar.xz archive from a source directory
    Q_INVOKABLE bool createLzmaArchive(const QString &sourceDir, const QString &archivePath);

Q_SIGNALS:
    void extractionCompleted(bool success);
    void compressionCompleted(bool success);

    void errorOccurred(const QString &message);
};
