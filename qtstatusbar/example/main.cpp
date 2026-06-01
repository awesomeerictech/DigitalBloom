

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include "statusbar.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Material");

    qmlRegisterType<StatusBar>("StatusBar", 0, 1, "StatusBar");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
