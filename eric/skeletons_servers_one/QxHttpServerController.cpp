
// QxHttpServerController.cpp
// Full file with QxOrm initialization, SQLite configuration, automatic table creation,
// SSL copying, route setup and server start/stop.
// Drop this over your existing QxHttpServerController.cpp (adjust includes paths if needed).

#include "QxHttpServerController.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QSslCertificate>
#include <QSslKey>

#include <QThread>

#include <memory>
#include <thread>

// QxOrm / QxService headers (adjust include paths to your project)
#include <QxRegister/QxClassX.h>
#include <QxService/QxConnect.h>
#include <QxService/QxThreadPool.h>

// QxOrm implementation umbrella (pulls qx::dao symbols). If your tree organizes differently,
// replace with the appropriate header(s). You previously had QxOrm_Impl.h available.
#include <QxOrm_Impl.h>

// Model includes (make sure these match your actual model header names)
// Your model includes (keep as you had them)
#include "orm/orders.h"
#include "orm/locations.h"
#include "orm/logistics.h"
#include "orm/delivery.h"
#include "orm/ids.hpp"
#include "orm/cryptocenter.hpp"
#include "orm/mysecrets.h"
#include "orm/metrics.hpp"
#include "orm/categories.hpp"


#include "MyJsonTools.hpp"
#include "sqliteone.hpp"

QVariantMap mysecretstable(qx::QxHttpRequest & request);

// Forward declare Qx types used in signatures (if headers don't already)
namespace qx { class QxHttpServer; }
namespace qx { namespace service { class QxConnect; } }

// -----------------------------------------------------------------------------
// Helper: execute SQL script loaded from resource (simple semicolon split)
// -----------------------------------------------------------------------------
bool QxHttpServerController::executeSqlScript(QSqlDatabase &db, const QString &scriptContents)
{
    QStringList statements = scriptContents.split(';', Qt::SkipEmptyParts);
    QSqlQuery q(db);
    for (QString stmt : statements) {
        stmt = stmt.trimmed();
        if (stmt.isEmpty()) continue;
        if (!q.exec(stmt)) {
            qWarning() << "[SQL] Statement failed:" << q.lastError().text()
                       << "\nSQL (truncated):" << stmt.left(2000);
            return false;
        } else {
            qInfo() << "[SQL] Executed statement (len)" << stmt.length();
        }
    }
    return true;
}


// -----------------------------------------------------------------------------
// Constructor / destructor
// -----------------------------------------------------------------------------
QxHttpServerController::QxHttpServerController(QObject *parent)
    : QObject(parent)
{
}

QxHttpServerController::~QxHttpServerController()
{
    stopServer();
}

// -----------------------------------------------------------------------------
// Utility: available IP addresses
// -----------------------------------------------------------------------------
QStringList QxHttpServerController::availableIpAddresses() const
{
    QStringList ips;
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            QHostAddress addr = entry.ip();
            if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
                ips << addr.toString();
            }
        }
    }
    ips.removeDuplicates();
    if (!ips.contains(QStringLiteral("127.0.0.1")))
        ips.prepend(QStringLiteral("127.0.0.1"));
    return ips;
}

void QxHttpServerController::setPort(int p)
{
    if (p == port_) return;
    port_ = p;
    Q_EMIT portChanged();
}

// -----------------------------------------------------------------------------
// SQLite configuration: pragmas + optional resource schema
// -----------------------------------------------------------------------------
bool QxHttpServerController::configureSqlite()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    QString base = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#else
    QString base = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif
    if (base.isEmpty()) base = QDir::currentPath();

    QDir().mkpath(base);
    sqlitePath_ = QDir(base).filePath("deliva.db");

    qInfo() << "[QxHttpServerController] SQLite DB file:" << sqlitePath_;

    // Short-lived main connection for pragmas and applying resource schema
    const QString connName = QStringLiteral("qx_http_main");
    if (QSqlDatabase::contains(connName)) QSqlDatabase::removeDatabase(connName);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
    db.setDatabaseName(sqlitePath_);
    if (!db.open()) {
        qWarning() << "[QxHttpServerController] Failed to open SQLite DB:" << db.lastError().text();
        QSqlDatabase::removeDatabase(connName);
        return false;
    }

    QSqlQuery q(db);
    if (!q.exec("PRAGMA journal_mode = WAL;")) {
        qWarning() << "[QxHttpServerController] PRAGMA journal_mode WAL failed:" << q.lastError().text();
    } else {
        qInfo() << "[QxHttpServerController] PRAGMA journal_mode = WAL";
    }
    if (!q.exec("PRAGMA synchronous = NORMAL;"))
        qWarning() << "[QxHttpServerController] PRAGMA synchronous NORMAL failed:" << q.lastError().text();
    if (!q.exec("PRAGMA temp_store = MEMORY;"))
        qWarning() << "[QxHttpServerController] PRAGMA temp_store MEMORY failed:" << q.lastError().text();

    // Prefer resource SQL if provided
    const QString resourcePath = QStringLiteral(":/sql/sqlite_schema.sql");
    if (QFile::exists(resourcePath)) {
        qInfo() << "[QxHttpServerController] Found packaged sqlite_schema.sql; applying schema...";
        QFile f(resourcePath);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "[QxHttpServerController] Failed opening resource" << resourcePath << ":" << f.errorString();
        } else {
            QTextStream in(&f);
            const QString scriptContents = in.readAll();
            f.close();
            if (!executeSqlScript(db, scriptContents)) {
                qWarning() << "[QxHttpServerController] Failed to execute schema script from resource";
            } else {
                qInfo() << "[QxHttpServerController] Schema script applied successfully";
            }
        }
    } else {
        qInfo() << "[QxHttpServerController] No packaged sqlite_schema.sql found in resources; will rely on qx::dao::create_table calls to create tables";
    }

    db.close();
    QSqlDatabase::removeDatabase(connName);
    return true;
}

// -----------------------------------------------------------------------------
// SSL configuration: copy resources and wire into QxConnect
// -----------------------------------------------------------------------------
bool QxHttpServerController::configureSsl(const QString &appPath)
{
    qInfo() << "[SSL] configureSsl: appPath =" << appPath;

    const QString resCA  = QStringLiteral(":/encrypt/DonaterootCA.crt");
    const QString resCrt = QStringLiteral(":/encrypt/donate.crt");
    const QString resKey = QStringLiteral(":/encrypt/donate.key");
    const QString resDh  = QStringLiteral(":/encrypt/dhparam.pem"); // optional

    QDir dir(appPath);
    if (!dir.exists("files")) dir.mkpath("files");

    const QString outCA  = dir.filePath("files/DonaterootCA.crt");
    const QString outCrt = dir.filePath("files/donate.crt");
    const QString outKey = dir.filePath("files/donate.key");
    const QString outDh  = dir.filePath("files/dhparam.pem");

    auto copyRes = [](const QString &src, const QString &dst) -> bool {
        if (!QFile::exists(src)) {
            qWarning() << "[SSL] Resource missing:" << src;
            return false;
        }
        QFile dest(dst);
        if (dest.exists()) dest.remove();
        if (!QFile::copy(src, dst)) {
            qWarning() << "[SSL] Failed to copy" << src << "->" << dst;
            return false;
        }
        return true;
    };

    bool okCA = copyRes(resCA, outCA);
    bool okCrt = copyRes(resCrt, outCrt);
    bool okKey = copyRes(resKey, outKey);
    bool okDh = true;
    if (QFile::exists(resDh)) {
        okDh = copyRes(resDh, outDh);
    } else {
        qInfo() << "[SSL] dhparam.pem not present in resources; continuing without it";
    }

    if (!okCA || !okCrt || !okKey) {
        qWarning() << "[SSL] Required cert/key resources missing or failed to copy - SSL disabled";
        return false;
    }

    // read CA
    QFile fCA(outCA);
    if (!fCA.open(QIODevice::ReadOnly)) {
        qWarning() << "[SSL] Cannot open CA cert file:" << outCA << fCA.errorString();
        return false;
    }
    QList<QSslCertificate> caList;
    caList << QSslCertificate(fCA.readAll());
    fCA.close();

    // read server cert
    QFile fCrt(outCrt);
    if (!fCrt.open(QIODevice::ReadOnly)) {
        qWarning() << "[SSL] Cannot open server cert file:" << outCrt << fCrt.errorString();
        return false;
    }
    QSslCertificate serverCert(fCrt.readAll());
    fCrt.close();

    // read private key
    QFile fKey(outKey);
    if (!fKey.open(QIODevice::ReadOnly)) {
        qWarning() << "[SSL] Cannot open server key file:" << outKey << fKey.errorString();
        return false;
    }
    const QByteArray keyData = fKey.readAll();
    fKey.close();

    // If your key is encrypted, you need the passphrase. Remove passphrase or provide it securely.
    const QByteArray keyPassphrase = QByteArrayLiteral("ericpass"); // development only - do not hardcode in production
    QSslKey serverKey(keyData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, keyPassphrase);
    if (serverKey.isNull()) {
        qWarning() << "[SSL] Failed to parse private key. Is the passphrase correct or key format supported?";
        return false;
    }

    // Configure QxConnect
    qx::service::QxConnect *serverSettings = qx::service::QxConnect::getSingleton();
    if (!serverSettings) {
        qWarning() << "[SSL] QxConnect singleton not available";
        return false;
    }

    serverSettings->setSSLEnabled(true);
    serverSettings->setSSLCACertificates(caList);
    serverSettings->setSSLLocalCertificate(serverCert);
    serverSettings->setSSLPrivateKey(serverKey);

    if (okDh) {
        qInfo() << "[SSL] dhparam.pem copied to" << outDh;
        // If QxConnect supports an API to set dhparam path, call it here.
    }

    qInfo() << "[SSL] SSL configured (self-signed/dev CA). Clients must trust CA or use proper certs.";
    return true;
}

// -----------------------------------------------------------------------------
// Router setup (paste your dispatch blocks here)
// -----------------------------------------------------------------------------
void QxHttpServerController::setupRoutes(qx::QxHttpServer &http)
{

 // Paste your dispatch(...) blocks from main.cpp here.
    // Minimal example:
    // Start create new account

}

// -----------------------------------------------------------------------------
// Start server
// -----------------------------------------------------------------------------
bool QxHttpServerController::startServer()
{
    if (running_) {
        qInfo() << "[QxHttpServerController] startServer: already running";
        return false;
    }

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    QString appPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#else
    QString appPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif
    if (appPath.isEmpty()) appPath = QDir::currentPath();
    QDir().mkpath(appPath);

    // Prepare SQLite (pragmas + optional resource schema)
    if (!configureSqlite()) {
        Q_EMIT errorOccurred("SQLite configuration failed");
        return false;
    }

    // Configure SSL (optional) - if it fails we log and continue without SSL
    if (!configureSsl(appPath)) {
        qWarning() << "[QxHttpServerController] configureSsl() failed - continuing without SSL";
    }

    // Expose URL for UI
    url_ = QStringLiteral("http://0.0.0.0:%1/").arg(port_);
    Q_EMIT urlChanged();

    running_ = true;

    // Start server thread
    thread_ = std::thread([this, appPath]() {
        qInfo() << "[QxHttpServerController] Server thread starting";

        // Create QxHttpServer
        http_.reset(new qx::QxHttpServer());

        // Configure QxConnect global settings
        auto cfg = qx::service::QxConnect::getSingleton();
        if (cfg) {
            cfg->setIp("0.0.0.0");
            cfg->setPort(port_);
            cfg->setThreadCount(50);
            cfg->setKeepAlive(5000);
            cfg->setCompressData(true);
            // SSL info was already applied in configureSsl()
        } else {
            qWarning() << "[QxHttpServerController] QxConnect singleton not available";
        }

        // ---------------- QxOrm initialization & table creation ----------------
        qInfo() << "[QxHttpServerController] Initializing QxOrm engine";

        // Initialize introspection engine (recommended)
        qx::QxClassX::registerAllClasses();
        qInfo() << "[QxOrm] qx::QxClassX::registerAllClasses() called";

        // Debug / verification options
        if (qx::QxSqlDatabase::getSingleton()) {
            qx::QxSqlDatabase::getSingleton()->setVerifyOffsetRelation(true);
            qInfo() << "[QxOrm] setVerifyOffsetRelation(true)";

            qx::QxSqlDatabase::getSingleton()->setFormatSqlQueryBeforeLogging(true);
            qInfo() << "[QxOrm] setFormatSqlQueryBeforeLogging(true)";

            qx::QxSqlDatabase::getSingleton()->setAddSqlSquareBracketsForTableName(true);
            qx::QxSqlDatabase::getSingleton()->setAddSqlSquareBracketsForColumnName(true);
            qInfo() << "[QxOrm] setAddSqlSquareBracketsForTableName/ColumnName(true)";
        } else {
            qWarning() << "[QxOrm] QxSqlDatabase singleton is NULL (cannot set debug flags)";
        }

        // Trace relation init (intensive logs)
        qx::IxSqlRelation::setTraceRelationInit(true);
        qInfo() << "[QxOrm] qx::IxSqlRelation::setTraceRelationInit(true)";

        // Configure QxSqlDatabase singleton to point to our sqlite file
        qx::QxSqlDatabase *pDatabase = qx::QxSqlDatabase::getSingleton();
        if (pDatabase) {
            pDatabase->setDriverName("QSQLITE");
            pDatabase->setDatabaseName(sqlitePath_);
            pDatabase->setHostName("localhost");
            pDatabase->setUserName("");
            pDatabase->setPassword("");
            qInfo() << "[QxOrm] qx::QxSqlDatabase configured to" << sqlitePath_;
        } else {
            qWarning() << "[QxOrm] qx::QxSqlDatabase::getSingleton() returned null";
        }

        // Create tables using qx::dao::create_table<T>() for your domain classes
        qInfo() << "[QxOrm] Creating/Verifying SQLite tables via qx::dao::create_table<...>()";
        QSqlError daoErr;

        // NOTE: The template parameter must match your actual C++ class names from the headers.
        daoErr = qx::dao::create_table<categories>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<categories>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for categories";

        daoErr = qx::dao::create_table<cryptocenter>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<cryptocenter>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for cryptocenter";

        daoErr = qx::dao::create_table<delivery>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<delivery>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for delivery";

        daoErr = qx::dao::create_table<ids>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<ids>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for ids";

        daoErr = qx::dao::create_table<items>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<items>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for items";

        daoErr = qx::dao::create_table<locations>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<locations>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for location";

        daoErr = qx::dao::create_table<logistics>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<logistics>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for logistics";

        daoErr = qx::dao::create_table<metrics>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<metrics>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for metrics";

        daoErr = qx::dao::create_table<mysecrets>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<mysecrets>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for mysecrets";

        daoErr = qx::dao::create_table<orders>();
        if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<orders>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for orders";

        qInfo() << "[QxOrm] Table creation phase complete";

        // -----------------------------------------------------------------------

        // Setup routes (paste your dispatch blocks into setupRoutes)
        setupRoutes(*http_);

        // Open per-thread SQLite connection for the server thread (handlers can use this)
        QString connName = QStringLiteral("qx_http_thread_%1").arg((quintptr)QThread::currentThreadId());
        if (QSqlDatabase::contains(connName)) QSqlDatabase::removeDatabase(connName);

        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
            db.setDatabaseName(sqlitePath_);
            if (!db.open()) {
                qWarning() << "[QxHttpServerController] Thread DB open failed:" << db.lastError().text();
            } else {
                QSqlQuery q(db);
                q.exec("PRAGMA journal_mode = WAL;");
            }

            qInfo() << "[QxHttpServerController] Starting http server (this call blocks until stop)";
            http_->startServer();
            qInfo() << "[QxHttpServerController] http_->startServer() returned; server stopped";
            // db destroyed when leaving scope
        }

        // cleanup per-thread connection registry
        if (QSqlDatabase::contains(connName)) {
            QSqlDatabase::removeDatabase(connName);
            qInfo() << "[QxHttpServerController] Removed thread DB connection:" << connName;
        }

        // Notify UI that server stopped
        QMetaObject::invokeMethod(this, [this]() {
            running_ = false;
            Q_EMIT stopped();
            Q_EMIT runningChanged();
        }, Qt::QueuedConnection);
    });

    Q_EMIT started();
    Q_EMIT runningChanged();
    return true;
}

// -----------------------------------------------------------------------------
// Stop server
// -----------------------------------------------------------------------------
void QxHttpServerController::stopServer()
{
    if (!running_) {
        qInfo() << "[QxHttpServerController] stopServer: not running";
        return;
    }

    qInfo() << "[QxHttpServerController] stopServer: requesting server stop";
    try {
        if (http_) http_->stopServer();
    } catch (const std::exception &ex) {
        qWarning() << "[QxHttpServerController] stop exception:" << ex.what();
        Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
    } catch (...) {
        qWarning() << "[QxHttpServerController] stop unknown exception";
        Q_EMIT errorOccurred("Unknown exception while stopping server");
    }

    if (thread_.joinable()) thread_.join();
    http_.reset();
    running_ = false;
    Q_EMIT stopped();
    Q_EMIT runningChanged();
}

// -----------------------------------------------------------------------------
// Example helper function you already had (kept intact)
// -----------------------------------------------------------------------------
QVariantMap mysecretstable(qx::QxHttpRequest & request) {



    std::string thesecretjson = R"(

    {
        "action": "fetch_by_query",
        "data": {
            "username": "Deliva"
        },
        "entity": "mysecrets",
        "query": {
            "sql": "username = 'Deliva'"
        },
        "relations": [
            "*"
        ]
    }

    )";



    QString strPayloadsecrets = QString::fromStdString(thesecretjson);
    std::shared_ptr<MyJsonTools> mypayloadsecrets {new MyJsonTools};
    mypayloadsecrets->processRequest(strPayloadsecrets);
    QJsonObject mydearpayloadsecrets = mypayloadsecrets->m_requestJson.toObject();
    //   QString myentitysecrets =  mydearpayloadsecrets.value("entity").toString();
    //  QString myactionsecrets =  mydearpayloadsecrets.value("action").toString();
    QJsonValue myvaluesecrets = mydearpayloadsecrets .value("data");
    QJsonObject myrequestsecrets = myvaluesecrets.toObject();
    QString myusernamesecrets =  myrequestsecrets.value("username").toString();
    qInfo() << "myusernamesecrets is: " << myusernamesecrets << Qt::endl;
    QMap<QString, QVariant> params0;
    params0["username"] = myusernamesecrets;

    QString where0 = buildWhereClause(params0,QSet<QString>(),"mysecrets");
    // Example: username = 'secret_user'

    qInfo() << "where0 is: " << where0 << Qt::endl;

    QJsonDocument userdoc = QJsonDocument::fromJson(strPayloadsecrets.toUtf8());
    std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
    modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
    modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
    modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("mysecrets"));
    qx::QxRestApi cryptoapi;
    modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where0);
    QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
    //  qDebug() << "adsupwebjson mysecrets after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
    qx::QxRestApi cryptoapiadsupwebjson;
    QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
    //  qDebug() << "result after query in resultcryptoadsupwebjson secrets is: " << resultcryptoadsupwebjson << Qt::endl;
    mypayloadsecrets->processRequest(resultcryptoadsupwebjson);
    QJsonObject mydearpayloadresultsadsupwebjson = mypayloadsecrets->m_requestJson.toObject();
    QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
    QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
    QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
    QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
    QString queryresultssecretid =  myadsupwebjsonfromquery.value("mysecrets_id").toString();
    QString queryresultssecretusername =  myadsupwebjsonfromquery.value("username").toString();
    QString queryresultssecretregister =  myadsupwebjsonfromquery.value("secretregister").toString();
    QString queryresultssecretlogin =  myadsupwebjsonfromquery.value("secretlogin").toString();
    QString queryresultssecretdate =  myadsupwebjsonfromquery.value("created_at").toString();
    QVariantMap mydearsecrets;
    mydearsecrets.insert("mysecrets_id", queryresultssecretid);
    mydearsecrets.insert("secretregister", queryresultssecretregister);
    mydearsecrets.insert("secretlogin", queryresultssecretlogin);
    mydearsecrets.insert("username", queryresultssecretusername);
    mydearsecrets.insert("created_at", queryresultssecretdate);

    return mydearsecrets;


}





