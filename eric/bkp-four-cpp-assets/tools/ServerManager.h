#pragma once
#include <QObject>
#include <memory>

class StreamServer;
class QxHttpServerController;
class ServerController;
class QQmlContext;

class ServerManager : public QObject {
    Q_OBJECT
public:
    static ServerManager* instance(); // singleton accessor

    explicit ServerManager(QObject* parent = nullptr);
    ~ServerManager();

    Q_INVOKABLE void startAllServers(); // start StreamServer then others
    Q_INVOKABLE void stopAllServers();

    // QML registration helper
    static void registerQmlTypes();
    void mycontext(QQmlContext* context);

private:
    void onStreamStarted();

    StreamServer* stream_ = nullptr;
    QxHttpServerController* qxctrl_ = nullptr;
    ServerController* wtctrl_ = nullptr;
    QQmlContext* mycontext_ = nullptr;
};
ServerManager* serverManagerInstance();
