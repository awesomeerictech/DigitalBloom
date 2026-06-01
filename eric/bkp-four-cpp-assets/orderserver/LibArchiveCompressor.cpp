// LibArchiveCompressor.cpp
#include "LibArchiveCompressor.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <functional>

#include <archive.h>
#include <archive_entry.h>

// Portable mapping from QFile::Permissions to numeric mode bits (octal-style).
static unsigned int qtPermissionsToMode(QFile::Permissions perms, bool isDir)
{
    unsigned int mode = 0;

    // owner
    if (perms & QFileDevice::ReadOwner)  mode |= 0400u;
    if (perms & QFileDevice::WriteOwner) mode |= 0200u;
    if (perms & QFileDevice::ExeOwner)   mode |= 0100u;
    // group
    if (perms & QFileDevice::ReadGroup)  mode |= 0040u;
    if (perms & QFileDevice::WriteGroup) mode |= 0020u;
    if (perms & QFileDevice::ExeGroup)   mode |= 0010u;
    // others
    if (perms & QFileDevice::ReadOther)  mode |= 0004u;
    if (perms & QFileDevice::WriteOther) mode |= 0002u;
    if (perms & QFileDevice::ExeOther)   mode |= 0001u;

    if (mode == 0) {
        // reasonable defaults
        mode = isDir ? 0755u : 0644u;
    }
    return mode;
}

// Try to add a compression filter in order of preference.
// Returns true if a filter was added and sets chosenExt to something like ".tar.xz"
static bool add_best_available_filter(struct archive *a, QString &chosenExt)
{
    if (archive_write_add_filter_xz(a) == ARCHIVE_OK) {
        chosenExt = ".tar.xz";
        return true;
    }
    if (archive_write_add_filter_lzma(a) == ARCHIVE_OK) {
        chosenExt = ".tar.lzma";
        return true;
    }
    if (archive_write_add_filter_gzip(a) == ARCHIVE_OK) {
        chosenExt = ".tar.gz";
        return true;
    }
    if (archive_write_add_filter_bzip2(a) == ARCHIVE_OK) {
        chosenExt = ".tar.bz2";
        return true;
    }
    chosenExt.clear();
    return false;
}

bool LibArchiveCompressor::createTarXzFromFolder(const QString &folderPath, const QString &outputArchivePath)
{
    QFileInfo srcInfo(folderPath);
    if (!srcInfo.exists() || !srcInfo.isDir()) {
        qWarning() << "createTarXzFromFolder: source invalid:" << folderPath;
        return false;
    }

    // Prepare output directory.
    QString outPath = outputArchivePath;
    QFileInfo outInfo(outPath);
    QString outDir = outInfo.path();
    if (outDir.isEmpty()) outDir = ".";

    if (!QDir().mkpath(outDir)) {
        qWarning() << "createTarXzFromFolder: failed to create output directory:" << outDir;
        return false;
    }

    struct archive *a = archive_write_new();
    if (!a) {
        qWarning() << "archive_write_new failed";
        return false;
    }

    QString chosenExt;
    bool filterAdded = add_best_available_filter(a, chosenExt);
    if (!filterAdded) {
        qWarning() << "No compression filters available in libarchive.";
        archive_write_free(a);
        return false;
    }

    // TAR format
    if (archive_write_set_format_pax_restricted(a) != ARCHIVE_OK) {
        qWarning() << "archive_write_set_format_pax_restricted failed:" << archive_error_string(a);
        archive_write_free(a);
        return false;
    }

    // Adjust extension if needed.
    if (!outPath.endsWith(chosenExt, Qt::CaseInsensitive)) {
        if (outPath.contains(".tar", Qt::CaseInsensitive)) {
            int idx = outPath.toLower().indexOf(".tar");
            outPath = outPath.left(idx) + chosenExt;
        } else {
            outPath += chosenExt;
        }
    }

    QByteArray outUtf8 = outPath.toUtf8();
    if (archive_write_open_filename(a, outUtf8.constData()) != ARCHIVE_OK) {
        qWarning() << "archive_write_open_filename failed:" << archive_error_string(a)
        << "path:" << outPath;
        archive_write_free(a);
        return false;
    }

    QString baseName = srcInfo.fileName();
    QDir srcDir(folderPath);

    std::function<bool(const QString&)> addPath;
    addPath = [&](const QString &absolutePath) -> bool {
        QFileInfo fi(absolutePath);

        QString rel = srcDir.relativeFilePath(absolutePath);
        if (rel == "." || rel.isEmpty()) rel.clear();

        QString archivePath = rel.isEmpty() ? baseName : (baseName + "/" + rel);
        archivePath.replace('\\', '/');

        struct archive_entry *entry = archive_entry_new();
        if (!entry) {
            qWarning() << "archive_entry_new failed";
            return false;
        }

        archive_entry_set_pathname(entry, archivePath.toUtf8().constData());

        unsigned int mode = qtPermissionsToMode(fi.permissions(), fi.isDir());
        archive_entry_set_perm(entry, mode);

        archive_entry_set_uid(entry, 0);
        archive_entry_set_gid(entry, 0);
        archive_entry_set_mtime(entry, fi.lastModified().toSecsSinceEpoch(), 0);

        if (fi.isDir()) {
            archive_entry_set_filetype(entry, AE_IFDIR);
            archive_entry_set_size(entry, 0);
            int r = archive_write_header(a, entry);
            archive_entry_free(entry);
            if (r != ARCHIVE_OK) {
                qWarning() << "archive_write_header(dir) failed for" << archivePath
                           << ":" << archive_error_string(a);
                return false;
            }
            return true;
        } else if (fi.isSymLink()) {
            QString target = fi.symLinkTarget();
            archive_entry_set_filetype(entry, AE_IFLNK);
            archive_entry_set_symlink(entry, target.toUtf8().constData());
            archive_entry_set_size(entry, 0);
            int r = archive_write_header(a, entry);
            archive_entry_free(entry);
            if (r != ARCHIVE_OK) {
                qWarning() << "archive_write_header(symlink) failed for" << archivePath
                           << ":" << archive_error_string(a);
                return false;
            }
            return true;
        } else if (fi.isFile()) {
            QFile f(absolutePath);
            if (!f.open(QIODevice::ReadOnly)) {
                qWarning() << "Failed to open file for reading:" << absolutePath;
                archive_entry_free(entry);
                return false;
            }

            qint64 fileSize = f.size();
            archive_entry_set_filetype(entry, AE_IFREG);
            archive_entry_set_size(entry, static_cast<la_int64_t>(fileSize));

            int r = archive_write_header(a, entry);
            if (r != ARCHIVE_OK) {
                qWarning() << "archive_write_header(file) failed for" << archivePath
                           << ":" << archive_error_string(a);
                f.close();
                archive_entry_free(entry);
                return false;
            }

            const qint64 BUF = 64 * 1024;
            while (!f.atEnd()) {
                QByteArray chunk = f.read(BUF);
                if (chunk.isEmpty()) break;

                la_ssize_t wrote = archive_write_data(a, chunk.constData(), static_cast<size_t>(chunk.size()));
                if (wrote < 0) {
                    qWarning() << "archive_write_data failed for" << archivePath
                               << ":" << archive_error_string(a);
                    f.close();
                    archive_entry_free(entry);
                    return false;
                }
            }

            f.close();
            archive_entry_free(entry);
            return true;
        }

        archive_entry_free(entry);
        return true;
    };

    // Add base folder
    if (!addPath(folderPath)) {
        archive_write_close(a);
        archive_write_free(a);
        return false;
    }

    // Recursively add contents
    QDirIterator it(folderPath,
                    QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden,
                    QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    while (it.hasNext()) {
        if (!addPath(it.next())) {
            archive_write_close(a);
            archive_write_free(a);
            return false;
        }
    }

    if (archive_write_close(a) != ARCHIVE_OK) {
        qWarning() << "archive_write_close failed:" << archive_error_string(a);
        archive_write_free(a);
        return false;
    }

    archive_write_free(a);

    qInfo() << "Created archive at" << outPath << "(compression:" << chosenExt << ")";
    return true;
}
