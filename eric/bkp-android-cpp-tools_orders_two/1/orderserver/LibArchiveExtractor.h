#ifndef LIBARCHIVEEXTRACTOR_H
#define LIBARCHIVEEXTRACTOR_H

#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>

class LibArchiveExtractor {
public:
    LibArchiveExtractor() = default;
    bool extractArchiveFromQrc(const QString &qrcFilePath, const QString &destinationDir);
    bool extractTarXzFromBuffer(const QByteArray &data, const QString &destinationDir);
    bool extractTarXzFromMemory(const void* data, size_t size, const QString& destination);
    QJsonObject extractAndProcessTarXzFile(const QString &sourceFilePath,
                                           const QString &destinationDir,
                                           const QString &username,
                                           const QString &baseName,
                                           const QString &category);
};

#endif // LIBARCHIVEEXTRACTOR_H
