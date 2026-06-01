#include <QCoreApplication>
#include <QDirIterator>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDebug>

void generateQrcFile(const QString &inputFolder, const QString &outputQrc) {
    QFile out(outputQrc);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Could not open output file:" << outputQrc;
        return;
    }

    QXmlStreamWriter xml(&out);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("RCC");
    xml.writeStartElement("qresource");
    xml.writeAttribute("prefix", "/serverassets");

    QDir inputDir(inputFolder);
    QDirIterator it(inputFolder, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString fullPath = it.next();
        QString relativePath = inputDir.relativeFilePath(fullPath);
        xml.writeTextElement("file", relativePath.replace("\\", "/"));
    }

    xml.writeEndElement(); // </qresource>
    xml.writeEndElement(); // </RCC>
    xml.writeEndDocument();

    qInfo() << "QRC file generated at:" << QDir(outputQrc).absolutePath();
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // ❗ Change this path to the directory you want to include in the .qrc
    QString inputDir = "C:/Qt/dev/orderserver/DigitalBloom/eric/App";
    QString outputQrc = "serverassets.qrc";

    generateQrcFile(inputDir, outputQrc);
    return 0;
}
