// LibArchiveQmlBridge.cpp
#include "LibArchiveQmlBridge.h"

// Include your original extractor header (updated)
#include "LibArchiveExtractor.h"
#include "LibArchiveCompressor.h" // Assuming this exists for compressFolder

#include <QVariant>
#include <QByteArray>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QUrl>
#include <QStandardPaths> // Needed for standard location

LibArchiveQmlBridge *LibArchiveQmlBridge::instance = nullptr;

LibArchiveQmlBridge *LibArchiveQmlBridge::getInstance()
{
    if (instance == nullptr)
    {
        instance = new LibArchiveQmlBridge();
    }

    return instance;
}


LibArchiveQmlBridge::LibArchiveQmlBridge(QObject *parent)
    : QObject(parent)
{
}


bool LibArchiveQmlBridge::extractTarXzFromBuffer(const QByteArray &data, const QString &destinationDir)
{
    LibArchiveExtractor extractor;
    return extractor.extractTarXzFromBuffer(data, destinationDir);
}

bool LibArchiveQmlBridge::extractTarXzFromMemory(const QVariant &bufferVariant, const QString &destinationDir)
{
    QByteArray data;
    if (bufferVariant.canConvert<QByteArray>()) {
        data = bufferVariant.toByteArray();
    } else if (bufferVariant.type() == QVariant::String) {
        const QString s = bufferVariant.toString();
        data = QByteArray::fromBase64(s.toUtf8());
    } else {
        qWarning() << "extractTarXzFromMemory: unsupported variant type";
        return false;
    }

    LibArchiveExtractor extractor;
    // Prefer existing buffer API if available
    if (extractor.extractTarXzFromBuffer(data, destinationDir)) return true;
    // fallback to memory API if original class provides it
    return extractor.extractTarXzFromMemory(data.constData(), static_cast<size_t>(data.size()), destinationDir);
}

bool LibArchiveQmlBridge::compressFolder(const QString &folderPath, const QString &outputDir, const QString &archiveName)
{
    QFileInfo src(folderPath);
    if (!src.exists() || !src.isDir()) {
        qWarning() << "compressFolder: invalid source:" << folderPath;
        return false;
    }

    QString outDir = outputDir;
    if (outDir.isEmpty()) {
        outDir = src.path();
    }
    if (!QDir().mkpath(outDir)) {
        qWarning() << "compressFolder: failed to create output directory:" << outDir;
        return false;
    }

    QString archiveFile = archiveName;
    if (archiveFile.isEmpty()) {
        archiveFile = src.fileName() + ".tar.xz";
    } else if (!archiveFile.endsWith(".tar.xz", Qt::CaseInsensitive)) {
        archiveFile += ".tar.xz";
    }

    QString outPath = QDir(outDir).filePath(archiveFile);

    LibArchiveCompressor compressor;
    bool ok = compressor.createTarXzFromFolder(folderPath, outPath);
    if (!ok) {
        qWarning() << "compressFolder: createTarXzFromFolder failed";
        return false;
    }

    qInfo() << "compressFolder: created" << outPath;
    return true;
}

bool LibArchiveQmlBridge::extractTarXzFromQrc(const QString &qrcResourcePath, const QString &destinationDir)
{
    if (qrcResourcePath.isEmpty()) {
        qWarning() << "extractTarXzFromQrc: empty qrc path";
        return false;
    }

    QFile qrcFile(qrcResourcePath);
    if (!qrcFile.exists()) {
        qWarning() << "extractTarXzFromQrc: QRC resource missing:" << qrcResourcePath;
        return false;
    }

    if (!qrcFile.open(QIODevice::ReadOnly)) {
        qWarning() << "extractTarXzFromQrc: failed to open resource:" << qrcResourcePath
                   << " error:" << qrcFile.errorString();
        return false;
    }

    QByteArray data = qrcFile.readAll();
    qrcFile.close();

    qInfo() << "extractTarXzFromQrc: read size(bytes):" << data.size();
    if (data.isEmpty()) {
        qWarning() << "extractTarXzFromQrc: resource empty or not packaged:" << qrcResourcePath;
        return false;
    }

    // Use your existing extractor method (exact signature you provided)
    LibArchiveExtractor extractor;
    bool ok = extractor.extractTarXzFromMemory(data.constData(), static_cast<size_t>(data.size()), destinationDir);
    if (!ok) {
        qWarning() << "extractTarXzFromQrc: extraction failed for dest:" << destinationDir;
    } else {
        qInfo() << "extractTarXzFromQrc: extracted" << qrcResourcePath << "to" << destinationDir;
    }
    return ok;
}

bool LibArchiveQmlBridge::extractTarXzFromMemoryVariant(const QVariant &bufferVariant, const QString &destinationDir)
{
    QByteArray data;

    // Accept QByteArray (if passed from C++) or a base64 QString (convenient from QML)
    if (bufferVariant.canConvert<QByteArray>()) {
        data = bufferVariant.toByteArray();
    } else if (bufferVariant.type() == QVariant::String) {
        QString s = bufferVariant.toString();
        data = QByteArray::fromBase64(s.toUtf8());
    } else {
        qWarning() << "extractTarXzFromMemoryVariant: unsupported variant type";
        return false;
    }

    if (data.isEmpty()) {
        qWarning() << "extractTarXzFromMemoryVariant: empty buffer";
        return false;
    }

    LibArchiveExtractor extractor;
    bool ok = extractor.extractTarXzFromMemory(data.constData(), static_cast<size_t>(data.size()), destinationDir);
    if (!ok) qWarning() << "extractTarXzFromMemoryVariant: extraction failed for dest:" << destinationDir;
    return ok;
}

// NEW IMPLEMENTATION: Bridge function to call the complex extraction and processing logic
QJsonObject LibArchiveQmlBridge::processExtractedArchive(const QString &fileUrl,
                                                         const QString &username,
                                                         const QString &baseName,
                                                         const QString &category)
{
    // 1. Convert QML file URL (e.g., "file:///path/to/archive.tar.xz") to local path
    QUrl url(fileUrl);
    QString filePath = url.toLocalFile();

    if (filePath.isEmpty()) {
        qWarning() << "processExtractedArchive: Invalid file URL or file not found:" << fileUrl;
        return QJsonObject();
    }

    // 2. Determine the destination directory
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    const QString subfolder = "/Orders_files/App/media";
    QString destinationDir = downloadPath + subfolder;

    // Ensure the destination directory exists
    // QDir dir;
    // if (!dir.mkpath(destinationDir)) {
    //     qWarning() << "processExtractedArchive: Failed to create destination directory:" << destinationDir;
    //     return QJsonObject();
    // }

    qInfo() << "Processing archive:" << filePath << "to destination:" << destinationDir;

    // 3. Call the core extraction and JSON generation function
    LibArchiveExtractor extractor;
    QJsonObject resultJson = extractor.extractAndProcessTarXzFile(
        filePath,
        destinationDir,
        username,
        baseName,
        category
        );

    if (resultJson.isEmpty()) {
        qWarning() << "processExtractedArchive: Extraction and JSON generation failed.";
    } else {
        qInfo() << "processExtractedArchive: Successfully created JSON with extracted data.";
    }

    return resultJson;
}
