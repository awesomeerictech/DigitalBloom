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
    Q_PROPERTY(QString listenIpV6 READ listenIpV6 NOTIFY listenIpV6Changed)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

public:
    explicit StreamServer(QObject *parent = nullptr);
    ~StreamServer() override;

    // lifecycle
    Q_INVOKABLE bool startServer(); // non-blocking: spawns internal thread
    Q_INVOKABLE void stopServer();

    // networking helpers exposed to QML
    Q_INVOKABLE QStringList getLocalIps(bool includeV6 = false);
    Q_INVOKABLE QString getLocalIp();
    Q_INVOKABLE QString getLocalIpV6();
    Q_INVOKABLE QStringList availableNetworkIps(bool includeV6 = false); // convenience alias for QML/UI

    // configuration setters
    Q_INVOKABLE void setListenIp(const QString &ip);
    Q_INVOKABLE void setPort(int p);

    bool running() const { return running_.load(); }
    QString listenIp() const { return listenIp_; }
    QString listenIpV6() const { return listenIpV6_; }
    int port() const { return port_; }

Q_SIGNALS:
    void started();
    void stopped();
    void runningChanged();
    void listenIpChanged();
    void listenIpV6Changed();
    void portChanged();
    void errorOccurred(const QString &msg);

private:
    void threadMain();
    QString downloadLocation();
    QString computeIniPath();
    QString computeSslPath();

    // wrapper handle (C API)
    zlm_handle_t* zlm_ = nullptr;

    // worker thread that runs Qt-side orchestration (calls wrapper C API)
    std::unique_ptr<std::thread> thr_;

    // state
    std::atomic_bool running_{false};
    std::atomic_bool stopRequested_{false};

    // network config (exposed to QML via properties)
    QString listenIp_;
    QString listenIpV6_;
    int port_ = 0;
};
