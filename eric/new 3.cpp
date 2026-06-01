#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "codedecodervideosink.h"

#include "QZXing.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
     QZXing::registerQMLTypes();

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
      qmlRegisterType<CodeDecoderVideoSink>("CodeDecoderVideoSink", 1, 0, "CodeDecoderVideoSink");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}