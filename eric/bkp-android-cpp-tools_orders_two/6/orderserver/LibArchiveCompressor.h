// LibArchiveCompressor.h
#pragma once

#include <QString>

class LibArchiveCompressor
{
public:
    LibArchiveCompressor() = default;
    ~LibArchiveCompressor() = default;

    // Create a .tar.xz archive (uses libarchive).
    // folderPath: path to folder to compress (must exist).
    // outputArchivePath: full path to resulting .tar.xz file (will be overwritten).
    // Returns true on success.
    bool createTarXzFromFolder(const QString &folderPath, const QString &outputArchivePath);
};
