#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <atomic>
#include <memory>
#include <thread>

extern "C" {
#include "zlm_wrapper_api.h"
}

class StreamServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString listenIp READ listenIp NOTIFY listenIpChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

public:
    explicit StreamServer(QObject *parent = nullptr);
    ~StreamServer() override;

    Q_INVOKABLE bool startServer(); // non-blocking: spawns internal thread
    Q_INVOKABLE void stopServer();

    Q_INVOKABLE QStringList getLocalIps(bool includeV6 = false);
    Q_INVOKABLE QString getLocalIp();
    Q_INVOKABLE QString getLocalIpV6();

    Q_INVOKABLE void setListenIp(const QString &ip);
    Q_INVOKABLE void setPort(int p);

    bool running() const { return running_.load(); }
    QString listenIp() const { return listenIp_; }
    int port() const { return port_; }

Q_SIGNALS:
    void started();
    void stopped();
    void runningChanged();
    void listenIpChanged();
    void portChanged();
    void errorOccurred(const QString &msg);

private:
    void threadMain();
    QString downloadLocation();
    QString computeIniPath();
    QString computeSslPath();

    zlm_handle_t* zlm_ = nullptr;
    std::unique_ptr<std::thread> thr_;
    std::atomic_bool running_{false};
    std::atomic_bool stopRequested_{false};

    QString listenIp_;
    int port_ = 0;
};
