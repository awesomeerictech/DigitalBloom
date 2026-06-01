#include "AssetExtractor.h"
#include "precompiled.h"

AssetExtractor::AssetExtractor(QObject *parent)
    : QObject(parent) {}

bool AssetExtractor::extractLzmaArchive(const QString &qrcPath, const QString &outputDir) {
    QFile inputFile(qrcPath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open archive for reading:" << qrcPath;
        Q_EMIT errorOccurred("Unable to open archive");
        return false;
    }

    QByteArray lzmaData = inputFile.readAll();
    inputFile.close();

    std::string bufferStr(lzmaData.constData(), static_cast<size_t>(lzmaData.size()));
    std::stringstream compressedStream(bufferStr);
    std::ostringstream decompressedTar;

    try {
        boost::iostreams::filtering_istream in;
        in.push(boost::iostreams::lzma_decompressor());
        in.push(compressedStream);
        decompressedTar << in.rdbuf();
    } catch (const std::exception &e) {
        qWarning() << "Decompression failed:" << e.what();
        Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
        return false;
    }

    QTemporaryFile tempTar;
    tempTar.setAutoRemove(false);
    if (!tempTar.open()) {
        qWarning() << "Failed to create temp tar file";
        Q_EMIT errorOccurred("Temp tar creation failed");
        return false;
    }
    tempTar.write(decompressedTar.str().c_str(), decompressedTar.str().size());
    tempTar.flush();
    tempTar.close();

    QDir().mkpath(outputDir);
    QStringList args = {"-xf", tempTar.fileName(), "-C", outputDir};
    int result = QProcess::execute("tar", args);

    if (result != 0) {
        qWarning() << "Tar extraction failed";
        Q_EMIT errorOccurred("Tar extraction failed");
        return false;
    }

    Q_EMIT extractionCompleted(true);
    return true;
}

bool AssetExtractor::createLzmaArchive(const QString &sourceDir, const QString &archivePath) {
    QString tempTar = archivePath + ".tmp.tar";
    QDir dir(sourceDir);
    if (!dir.exists()) {
        qWarning() << "Source directory does not exist:" << sourceDir;
        Q_EMIT errorOccurred("Source directory missing");
        return false;
    }

    QStringList args = {"-cf", tempTar, "-C", sourceDir, "."};
    if (QProcess::execute("tar", args) != 0) {
        qWarning() << "Tar creation failed.";
        Q_EMIT errorOccurred("Tar creation failed");
        return false;
    }

    QFile inputTar(tempTar);
    if (!inputTar.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open tar file for compression.";
        Q_EMIT errorOccurred("Tar file could not be opened");
        return false;
    }

    QByteArray tarData = inputTar.readAll();
    inputTar.close();

    std::ofstream out(archivePath.toStdString(), std::ios::binary);
    if (!out.is_open()) {
        qWarning() << "Failed to open output archive.";
        Q_EMIT errorOccurred("Could not open archive for writing");
        return false;
    }

    try {
        boost::iostreams::filtering_ostream compressor;
        compressor.push(boost::iostreams::lzma_compressor());
        compressor.push(out);
        compressor.write(tarData.constData(), tarData.size());
        compressor.flush();
    } catch (const std::exception &e) {
        qWarning() << "Compression failed:" << e.what();
        Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
        return false;
    }

    QFile::remove(tempTar);
    Q_EMIT compressionCompleted(true);
    return true;
}
