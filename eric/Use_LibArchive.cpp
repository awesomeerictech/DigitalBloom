#include "AssetExtractor.h"

#include <QFile>
#include <QDir>
#include <QDebug>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/copy.hpp>

#include <archive.h>
#include <archive_entry.h>

AssetExtractor::AssetExtractor(QObject *parent)
    : QObject(parent) {}

bool AssetExtractor::extractLzmaArchive(const QString &qrcPath, const QString &destinationDir) {
    QFile archiveFile(qrcPath);
    if (!archiveFile.open(QIODevice::ReadOnly)) {
        emit errorOccurred(QStringLiteral("Failed to open archive: %1").arg(qrcPath));
        return false;
    }

    QByteArray rawData = archiveFile.readAll();

    struct archive *a = archive_read_new();
    archive_read_support_filter_lzma(a);
    archive_read_support_format_tar(a);

    struct archive_entry *entry;
    int r;

    r = archive_read_open_memory(a, rawData.data(), rawData.size());
    if (r != ARCHIVE_OK) {
        emit errorOccurred(QString::fromUtf8(archive_error_string(a)));
        archive_read_free(a);
        return false;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        QString filePath = destinationDir + QLatin1Char('/') + archive_entry_pathname(entry);
        QFileInfo info(filePath);
        if (archive_entry_filetype(entry) == AE_IFDIR) {
            QDir().mkpath(filePath);
        } else {
            QDir().mkpath(info.path());
            QFile outFile(filePath);
            if (!outFile.open(QIODevice::WriteOnly)) {
                emit errorOccurred(QStringLiteral("Failed to write file: %1").arg(filePath));
                continue;
            }
            const void *buff;
            size_t size;
            int64_t offset;
            while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
                outFile.write(static_cast<const char *>(buff), size);
            }
            outFile.close();
        }
    }

    archive_read_free(a);
    return true;
}

bool AssetExtractor::createTarLzmaArchive(const QString &sourceDir, const QString &outputPath) {
    // Optional: You can implement archive creation here using libarchive or boost if needed.
    emit errorOccurred(QStringLiteral("Archive creation not implemented yet."));
    return false;
}
