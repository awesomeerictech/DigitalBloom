#ifndef LIBARCHIVEEXTRACTOR_H
#define LIBARCHIVEEXTRACTOR_H

#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <archive.h> // Include libarchive header for struct archive

// Structure to hold metadata read from the archive
struct ArchiveMetadata {
    QStringList usernames;
    QStringList categories;
};

class LibArchiveExtractor {
public:
    LibArchiveExtractor() = default;

    // Existing extraction functions
    bool extractArchiveFromQrc(const QString &qrcFilePath, const QString &destinationDir);
    bool extractTarXzFromBuffer(const QByteArray &data, const QString &destinationDir);
    bool extractTarXzFromMemory(const void* data, size_t size, const QString& destination);

    /**
     * @brief Extracts media from a tar.xz file and generates JSON metadata.
     * * This function performs a two-pass operation: first to read metadata (.txt files)
     * from the archive, and second to extract media and generate the JSON.
     * * @param sourceFilePath Path to the source .tar.xz file.
     * @param destinationDir Directory where media files will be extracted.
     * @param baseName Name to use for the primary record.
     * @return QJsonObject containing the extracted metadata, or an empty object on failure.
     */
    QJsonObject extractAndProcessTarXzFile(const QString &sourceFilePath,
                                           const QString &destinationDir,
                                           const QString &baseName);

private:
    // Helper to read content of a specific file inside an open archive
    QByteArray readArchiveEntryContent(struct archive *a, struct archive_entry *entry);

    // First pass: finds and reads metadata from username.txt and category.txt
    ArchiveMetadata findMetadataInArchive(const QString &sourceFilePath);
};

#endif // LIBARCHIVEEXTRACTOR_H
