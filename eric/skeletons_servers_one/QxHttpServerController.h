#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include <thread>

#include <QxService/QxConnect.h> // adjust path if needed
#include <QxRegister/QxClassX.h> // for registerAllClasses

// Forward declare QxHttpServer to avoid heavy includes in header if you prefer
namespace qx { class QxHttpServer; }
namespace qx { namespace service { class QxConnect; } }

class QxHttpServerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString url READ url NOTIFY urlChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

public:
    explicit QxHttpServerController(QObject *parent = nullptr);
    ~QxHttpServerController() override;

    Q_INVOKABLE bool startServer();
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE QStringList availableIpAddresses() const;

    bool isRunning() const { return running_; }
    QString url() const { return url_; }
    int port() const { return port_; }
    void setPort(int p);

Q_SIGNALS:
    void started();
    void stopped();
    void errorOccurred(const QString &err);
    void runningChanged();
    void urlChanged();
    void portChanged();

private:
    bool configureSqlite();
    bool configureSsl(const QString &appPath);
    void setupRoutes(qx::QxHttpServer &http);

    // resource-based SQL helper
  //  static bool executeSqlScript(QSqlDatabase &db, const QString &scriptContents);
    bool executeSqlScript(QSqlDatabase &db, const QString &scriptContents);

private:
    std::unique_ptr<qx::QxHttpServer> http_;
    std::thread thread_;
    QString url_;
    QString sqlitePath_;
    bool running_ = false;
    int port_ = 880;  // default port
};
