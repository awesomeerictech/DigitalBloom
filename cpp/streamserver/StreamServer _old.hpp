#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include <thread>
#include <atomic>

namespace mediakit {
bool loadIniConfig(const char *ini_path = nullptr);
void initEventListener();
}
using namespace mediakit;

class StreamServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString listenIp READ listenIp NOTIFY listenIpChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

public:
    explicit StreamServer(QObject *parent = nullptr);
    ~StreamServer() override;

    // Control
    Q_INVOKABLE bool startServer(); // start using config (ini) or defaults
    Q_INVOKABLE void stopServer();

    // Network helpers
    Q_INVOKABLE QString getLocalIp();                       // preferred IPv4
    Q_INVOKABLE QString getLocalIpV6();                     // preferred IPv6
    Q_INVOKABLE QStringList getLocalIps(bool includeV6 = false); // candidates (v4/v6)
    Q_INVOKABLE void setListenIp(const QString &ip);       // set explicit listen IP (and restart if running)

    // Port accessor / mutator
    int port() const { return port_; }
    Q_INVOKABLE void setPort(int p);

    // Properties
    bool running() const { return running_.load(); }
    QString listenIp() const { return listenIp_; }

Q_SIGNALS:
    void started();
    void stopped();
    void runningChanged();
    void listenIpChanged();
    void portChanged();
    void errorOccurred(const QString &msg);

private:
    void threadMain();                     // server thread main
    QString resolveIniPath();              // downloads/exe fallback, returns utf8 path
    QString downloadLocation();            // Qt standard downloads path (platform-aware)

    std::unique_ptr<std::thread> thr_;
    std::atomic_bool running_{false};
    std::atomic_bool stopRequested_{false};

    // listen IP and port we will bind to — set by QML/C++
    QString listenIp_; // empty means "use mINI / defaults"
    int port_ = 0;     // 0 means "use mINI/defaults" (will be read on start)
};
