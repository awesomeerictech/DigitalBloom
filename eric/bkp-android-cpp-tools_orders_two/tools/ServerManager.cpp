#include "ServerManager.h"
#include "streamserver/StreamServer.hpp"
#include "ormserver/QxHttpServerController.h"
#include "orderserver/servercontrollerwt.h"
#include <QDebug>
#include <QCoreApplication>
#include <QMetaObject>
#include <QQmlEngine>
#include <QtQml>

static ServerManager* g_instance = nullptr;

ServerManager* ServerManager::instance()
{
    if (!g_instance) {
        g_instance = new ServerManager(qApp);
    }
    return g_instance;
}

ServerManager::ServerManager(QObject* parent)
    : QObject(parent)
{
    // don't create servers here (we create on startAllServers), but could register QML
}

ServerManager::~ServerManager()
{
   // stopAllServers();
    // cleanup
    // if (stream_) { delete stream_; stream_ = nullptr; }
    // if (qxctrl_) { delete qxctrl_; qxctrl_ = nullptr; }
    // if (wtctrl_) { delete wtctrl_; wtctrl_ = nullptr; }
}

ServerManager* serverManagerInstance() { return ServerManager::instance(); }

void ServerManager::registerQmlTypes()
{
    qmlRegisterType<StreamServer>("Local.Servers", 1, 0, "StreamServer");
    qmlRegisterType<QxHttpServerController>("Local.Servers", 1, 0, "QxHttpServerController");
    qmlRegisterType<ServerController>("Local.Servers", 1, 0, "ServerController");

}

void ServerManager::mycontext(QQmlContext* context) {

    mycontext_ = context;
}

void ServerManager::startAllServers()
{
    qDebug() << "ServerManager: startAllServers()";

    if (!stream_) {
        stream_ = new StreamServer(nullptr);
        // If StreamServer emits started(), connect it to onStreamStarted
        connect(stream_, &StreamServer::started, this, &ServerManager::onStreamStarted, Qt::QueuedConnection);
    } else {
        // already created: ensure proper restart logic if your classes support it
    }

    // Start the stream server first. Implementation must be non-blocking (internally spawn threads).
    QMetaObject::invokeMethod(stream_, "startServer", Qt::QueuedConnection);
}

void ServerManager::onStreamStarted()
{
    qDebug() << "ServerManager: stream started; creating and starting other servers";

    // Create other controllers only after stream is up so applyListenIpFrom works
    if (!qxctrl_) {
        qxctrl_ = new QxHttpServerController(nullptr);
        qxctrl_->setPort(9642);
    }
    if (!wtctrl_) {
        wtctrl_ = new ServerController(nullptr);
    }
    if((mycontext_ && stream_) && (qxctrl_ && wtctrl_) ) {

        mycontext_->setContextProperty("streamserver", stream_);
        mycontext_->setContextProperty("ormserver", qxctrl_);
        mycontext_->setContextProperty("orderserver", wtctrl_);

    }

    // Propagate discovered IP from stream to other controllers (assumes these methods exist)
    qxctrl_->applyListenIpFrom(stream_);
    wtctrl_->applyListenIpFrom(stream_);

    // Start them (call startServer; ensure their startServer is a Q_INVOKABLE or slot)
    QMetaObject::invokeMethod(qxctrl_, "startServer", Qt::QueuedConnection);
    QMetaObject::invokeMethod(wtctrl_, "startServer", Qt::QueuedConnection);
}

void ServerManager::stopAllServers()
{
    qDebug() << "ServerManager: stopAllServers()";
    // If your controller objects have stopServer methods use them, otherwise this is cleanup
    if (qxctrl_) {
        QMetaObject::invokeMethod(qxctrl_, "stopServer", Qt::QueuedConnection);
    }
    if (wtctrl_) {
        QMetaObject::invokeMethod(wtctrl_, "stopServer", Qt::QueuedConnection);
    }
    if (stream_) {
        QMetaObject::invokeMethod(stream_, "stopServer", Qt::QueuedConnection);
    }
}
