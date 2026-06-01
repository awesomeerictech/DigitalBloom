#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "androidsaver.h" // Include your header/source file

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // Register the C++ object with QML
    registerAndroidFileSaverType(&engine);

    const QUrl url(u"qrc:/YourAppName/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
