#pragma once

#include <QObject>
#include <memory>
#include <thread>
#include <atomic>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>


namespace Wt { class WServer; class WEnvironment; class WApplication; }

class ServerController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString url READ url NOTIFY urlChanged)
    Q_PROPERTY(QString assetPath READ assetPath NOTIFY assetPathChanged)
public:
    explicit ServerController(QObject *parent = nullptr);
    ~ServerController();

    // Start the Wt server on `port`. Copies assets to Downloads before starting.
    Q_INVOKABLE bool startServer(int port = 8180);

    // Stop the server and join the thread (blocking until stopped).
    Q_INVOKABLE void stopServer();

    Q_INVOKABLE QStringList availableIPAddresses();

    QString url() const { return m_url; }
    QString assetPath() const { return m_assetPath; }

Q_SIGNALS:
    void started();
    void stopped();
    void errorOccurred(const QString &msg);
    void urlChanged();
    void assetPathChanged();

private:
    std::unique_ptr<Wt::WServer> server_;
    std::unique_ptr<std::thread> serverThread_;
    std::atomic_bool running_{false};
    QString m_url;
    QString m_assetPath;
};
