
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
#include "streamserver/StreamServer.hpp"
#include "Util/logger.h"

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

void QxHttpServerController::applyListenIpFrom(StreamServer *server) {
    if (!server) {
        WarnL << "QxHttpServerController::applyListenIpFrom: null server pointer";
         qInfo() << "QxHttpServerController::applyListenIpFrom: null server pointer with ip: " << listenIp_ << Qt::endl;
        return;
    }

   // qInfo() << "applyListenIpFrom called with ip: " << listenIp_ << Qt::endl;

    // Prefer IPv4, fall back to IPv6
    QString ip4 = server->getLocalIp();
    QString ip6 = server->getLocalIpV6();
    QString chosen;
    if (!ip4.isEmpty()) chosen = ip4;
    else if (!ip6.isEmpty()) chosen = ip6;
    else {
        WarnL << "QxHttpServerController::applyListenIpFrom: StreamServer has no non-loopback IP";
        return;
    }

    qInfo() << "applyListenIpFrom ip4: " << ip4 << Qt::endl;
    qInfo() << "applyListenIpFrom ip6: " << ip6 << Qt::endl;
    qInfo() << "applyListenIpFrom chosen: " << chosen << Qt::endl;

    // validate (same as your existing checks)
    if (chosen.isEmpty() || chosen == QLatin1String("0.0.0.0") ||
        chosen == QLatin1String("::") || chosen == QLatin1String("::1") ||
        chosen.startsWith(QLatin1String("127."))) {
        WarnL << "QxHttpServerController::applyListenIpFrom: refused loopback/wildcard ip: " << chosen.toStdString();
        qInfo() << "Listen IP loopback is: " << listenIp_ << Qt::endl;
        return;
    }

    else {

        // store and apply
        listenIp_ = chosen;
        qInfo() << "Listen IP is: " << listenIp_ << Qt::endl;


    }


    InfoL << "QxHttpServerController::applyListenIpFrom: listenIp set to " << listenIp_.toStdString();

    // If controller is running, attempt restart using the reflection approach used previously.
    // bool restarted = false;
    // if (QMetaObject::invokeMethod(this, "stopServer")) {
    //     if (QMetaObject::invokeMethod(this, "startServer")) {
    //         InfoL << "QxHttpServerController::applyListenIpFrom: restarted using stopServer()/startServer.";
    //         restarted = true;
    //     } else if (QMetaObject::invokeMethod(this, "startServer")) {
    //         InfoL << "QxHttpServerController::applyListenIpFrom: restarted using stop()/startServer().";
    //         restarted = true;
    //     }
    // }
    // if (!restarted && QMetaObject::invokeMethod(this, "stopServer")) {
    //     if (QMetaObject::invokeMethod(this, "startServer")) {
    //         InfoL << "QxHttpServerController::applyListenIpFrom: restarted using stopServer()/startServer().";
    //         restarted = true;
    //     }
    // }
    // if (!restarted && QMetaObject::invokeMethod(this, "shutdown")) {
    //     if (QMetaObject::invokeMethod(this, "start")) {
    //         InfoL << "QxHttpServerController::applyListenIpFrom: restarted using shutdown()/start().";
    //         restarted = true;
    //     } else if (QMetaObject::invokeMethod(this, "startServer")) {
    //         InfoL << "QxHttpServerController::applyListenIpFrom: restarted using shutdown()/startServer().";
    //         restarted = true;
    //     }
    // }
    // if (!restarted) {
    //     InfoL << "QxHttpServerController::applyListenIpFrom: controller may be running; please restart manually to apply IP.";
    // }
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

   // QDir().mkpath(base);
   // sqlitePath_ = QDir(base).filePath("/Orders_files/App/deliva.db");
    QString relativeFilePath = "/Orders_files/App/deliva.db";

    // 3. Combine them using QDir::cleanPath for cross-platform compatibility
    sqlitePath_ = QDir::cleanPath(base + relativeFilePath);

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

    http.dispatch("POST", "/deliva/ids/new", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errlog.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/scclog.log");
        Wt::Json::Object bodyContent;
        Wt::Json::Object tokenContent;
        Wt::Json::parse(QString::fromUtf8(request.data()).toStdString(), tokenContent);
        std::string registerjwt = tokenContent.get("token");
        //std::cout << "register jwt is: " << registerjwt << std::endl;
        Wt::WLogEntry sccentry = scclogger.entry("info");
        //  std::unique_ptr<MyJsonTools> toolidsnew {new MyJsonTools};
        //  toolidsnew->processRequest(request.data());
        //  QJsonObject myrequest = toolidsnew->m_requestJson.toObject();
        // QString mytokendata = myrequest.value("token").toString();
        // qDebug() << "my token data is: " << mytokendata << Qt::endl;
        QString strToken = QString::fromStdString(registerjwt);
        QStringList listJwtParts = strToken.split(".");
        bool tokenformatcorrect = true;
        if (listJwtParts.count() != 3){
            // token format error
            tokenformatcorrect = false;
            errentry << "bad token";


        }

        QVariantMap extractsecrets = mysecretstable(request);
        qInfo() << "Variant Map for secret Table: " << extractsecrets << Qt::endl;
        QString strSecret =  extractsecrets.value("secretregister").toString();
        std::cout << "strSecret is: " << strSecret.toStdString() << std::endl;
        if (strSecret.isEmpty())
        {
          qDebug() << "strSecret is empty: " << strSecret << Qt::endl;
        }

        QJsonWebToken token = QJsonWebToken::fromTokenAndSecret(strToken, strSecret);
        bool isvalidtoken = false;
        // get decoded header and payload
        // QString strHeader = token.getHeaderQStr();
        QString strPayload = "";
        // QString jsonobj ="";

        if (token.isValid()) {

            strPayload = token.getPayloadQStr();
            qDebug() <<"valid token" << Qt::endl;
            isvalidtoken=true;


        }


        else {


            errentry << "invalid token";
            //  qDebug() <<"invalid token" << Qt::endl;
            std::cout << "isvalidtoken is: " << isvalidtoken << std::endl;
            std::cout << "tokenformatcorrect is: " << tokenformatcorrect << std::endl;





        }

         qDebug() << "Payload is: " << strPayload << Qt::endl;


        //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QJsonValue myvalue = mydearpayload.value("data");
        QString myaction =  mydearpayload.value("action").toString();
        QString myentity =  mydearpayload.value("entity").toString();
        QJsonObject myrequest = myvalue.toObject();
        QString myusername =  myrequest.value("username").toString();
        QString mypassword =  myrequest.value("password").toString();
        qint64 mypolicy =  myrequest.value("policy").toInt();

        if(isvalidtoken && tokenformatcorrect ) {



            QMap<QString,QVariant> p;
            p["username"] = myusername;
            QString where = buildWhereClause(p);

            QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
             qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
            std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where);
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", "fetch_by_query");
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", "cryptocenter");
            qx::QxRestApi queryapi;
            QString result = queryapi.processRequest(userdoc.toJson(QJsonDocument::JsonFormat::Compact));
            // qDebug() << "validate query json is: " << result << Qt::endl;
            //  const QJsonDocument mydoc = QJsonDocument::fromJson(result.toUtf8());
            // qDebug() << "results as json doc are: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Compact)) << Qt::endl;

            std::shared_ptr<MyJsonTools> mypayloadquery {new MyJsonTools};
            mypayloadquery->processRequest(result);
            QJsonObject mydearquery = mypayloadquery->m_requestJson.toObject();
            QJsonValue myvaluefromquery = mydearquery.value("data");
            QJsonArray myrequestfromqueryarr = myvaluefromquery.toArray();
            QJsonValue dataFirstarr = myrequestfromqueryarr.first();
            QJsonObject myrequestfromquery = dataFirstarr.toObject();
            QString queryresults =  myrequestfromquery.value("username").toString();
            //  qDebug() << "query response is: " << queryresults << Qt::endl;


            if(!(queryresults==myusername)) {

                qx::QxRestApi loadapi;
                QJsonDocument modifymydoc = QJsonDocument::fromJson(strPayload.toUtf8());
                std::shared_ptr<MyJsonTools>  modifymyMyJsonTools {new MyJsonTools};
                modifymyMyJsonTools->resetadmin(modifymydoc, modifymyMyJsonTools,2);
                // if((myentity=="ids" || myentity=="cryptocenter"  ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
                if((myentity=="mysecrets" ) ) {

                    modifymyMyJsonTools->modifyJsonValue(modifymydoc, "entity", QJsonValue("nullandvoid"));

                }

                QString myidsjson = modifymydoc.toJson(QJsonDocument::JsonFormat::Indented);
                //  qDebug() << "myidsjson after reset is: " << qPrintable(myidsjson) << Qt::endl;
                QString resultreg = loadapi.processRequest(myidsjson);
                //  qDebug() << "result after registration in ids is: " << resultreg << Qt::endl;


                QVariantMap mysecurity = modifymyMyJsonTools->encryptpass(mypassword.toStdString());
                QString mycryptokey = mysecurity.value("key").toString();
                modifymyMyJsonTools->modifyJsonValue(modifymydoc, "entity", QJsonValue("cryptocenter"));
                modifymyMyJsonTools->modifyJsonValue(modifymydoc, "query.entity", QJsonValue("cryptocenter"));

                if((myentity=="mysecrets" ) ) {

                    modifymyMyJsonTools->modifyJsonValue(modifymydoc, "entity", QJsonValue("nullandvoid"));
                    modifymyMyJsonTools->modifyJsonValue(modifymydoc, "query.entity", QJsonValue("nullandvoid"));

                }


                if(mypolicy==0) {


                    modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.key", QJsonValue(mycryptokey));

                }

                else {


                    modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.key", QJsonValue("policyretain"));


                }

                modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.iv",  QJsonValue(mysecurity.value("iv").toString()));
                modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.cipher",  QJsonValue(mysecurity.value("cipher").toString()));
                QRandomGenerator64 *gen = QRandomGenerator64::system();
                gen->securelySeeded();
                long long discardx = gen->bounded(1000000000);
                // qDebug() << "my token  is: " << discardx << Qt::endl;
                modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.token", QJsonValue(discardx));
                QString myfinaljson = modifymydoc.toJson(QJsonDocument::JsonFormat::Compact);
                //  QString myfinaljson = modifymydoc.toJson(QJsonDocument::JsonFormat::Indented);
                // qDebug() << "my final json after modification is: " << qPrintable(myfinaljson) << Qt::endl;

                qx::QxRestApi cryptoapi;
                QString resultcrypto = cryptoapi.processRequest(myfinaljson);
                Q_UNUSED(resultcrypto)
                //  qDebug() << "result after registration in crypto is: " << resultcrypto << Qt::endl;
                mypayload->processRequest(resultreg);
                QJsonObject myregpayload = mypayload->m_requestJson.toObject();
                QJsonValue myregvalue = myregpayload.value("data");
                QJsonObject myregrequest = myregvalue.toObject();
                QString myids_id =  myregrequest.value("ids_id").toString();
                // qDebug() << "my ids_id is: " << myids_id << Qt::endl;
                QJsonWebToken m_jwtObj;
                QDateTime mycurrent = QDateTime::currentDateTime();
                QString mydate = mycurrent.toString(Qt::ISODate);
                m_jwtObj.removeAll();
                m_jwtObj.appendClaim(myusername,  QString::number(discardx));
                m_jwtObj.setAlgorithmStr("HSSha3_512");
                m_jwtObj.setSecret(strSecret);
                QJsonArray newarr;
                QJsonObject object;
                object.insert("token", m_jwtObj.getToken());
                object.insert("ids_id", myids_id);
                if((mypolicy==1)) {

                    object.insert("key", mycryptokey);

                }
                object.insert("date", QString::fromStdString(mydate.toStdString()));
                newarr << object;
                QJsonDocument responsedoc(newarr);
                //  QString myresponsedocjson = responsedoc.toJson(QJsonDocument::JsonFormat::Indented);
                // qDebug() << "myresponsedocjson from register is: " << qPrintable(myresponsedocjson) << Qt::endl;
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);
                errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                         << myrequest.value("created_at").toString().toStdString() << " ." ;
                sccentry << " Registration of new user " << myrequest.value("username").toString().toStdString() <<" at "
                         << myrequest.value("created_at").toString().toStdString() << " was successful." ;





            }

            else {


                const std::string ErrorTemplate = "alreadyexists";
                QString result = (QString::fromStdString(ErrorTemplate));
                response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
                response.status() = 403;
                response.data() = result.toUtf8();
                sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                         << myrequest.value("created_at").toString().toStdString() << " ." ;
                errentry << " Registration of new user " << myrequest.value("username").toString().toStdString() <<" at "
                         << myrequest.value("created_at").toString().toStdString() << " was not successful since the user existed." ;


            }






        }

        else {

            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " Registration of new user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;




        }






    });

    // End create new account



    // Start login user

    http.dispatch("POST", "/deliva/ids/login", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {


        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errlog.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/scclog.log");
        Wt::Json::Object bodyContent;
        Wt::Json::Object tokenContent;
        Wt::Json::parse(QString::fromUtf8(request.data()).toStdString(), tokenContent);
        std::string loginjwt = tokenContent.get("token");
        //  std::cout << "login jwt is: " << loginjwt << std::endl;
        Wt::WLogEntry sccentry = scclogger.entry("info");
        //  std::unique_ptr<MyJsonTools> toolidsnew {new MyJsonTools};
        //  toolidsnew->processRequest(request.data());
        //  QJsonObject myrequest = toolidsnew->m_requestJson.toObject();
        // QString mytokendata = myrequest.value("token").toString();
        // qDebug() << "my token data is: " << mytokendata << Qt::endl;
        QString strToken = QString::fromStdString(loginjwt);
        QStringList listJwtParts = strToken.split(".");
        bool tokenformatcorrect = true;
        if (listJwtParts.count() != 3){
            // token format error
            tokenformatcorrect = false;
            errentry << "bad token";


        }

        QVariantMap extractsecrets = mysecretstable(request);
        QString strSecret =  extractsecrets.value("secretlogin").toString();
        if (strSecret.isEmpty())
        {

        }

        QJsonWebToken token = QJsonWebToken::fromTokenAndSecret(strToken, strSecret);
        bool isvalidtoken = false;
        // get decoded header and payload
        // QString strHeader = token.getHeaderQStr();
        QString strPayload = "";
        // QString jsonobj ="";

        if (token.isValid()) {

            strPayload = token.getPayloadQStr();
            // qDebug() <<"valid token" << Qt::endl;
            isvalidtoken=true;


        }


        else {


            errentry << "invalid token";
            //  qDebug() <<"invalid token" << Qt::endl;




        }


        //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myusername =  myrequest.value("username").toString();
        QString mypassword =  myrequest.value("password").toString();

        if(isvalidtoken && tokenformatcorrect ) {



            QMap<QString,QVariant> p;
            p["username"] = myusername;
            QString where = buildWhereClause(p);

            QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
            // qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
            std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where);
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
            // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
            QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
            qx::QxRestApi cryptoapi;
            QString resultcrypto = cryptoapi.processRequest(myfinaljson);
            Q_UNUSED(resultcrypto)
            //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
            mypayload->processRequest(resultcrypto);
            QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
            QJsonValue myregvalue = mydearpayloadresults.value("data");
            QJsonArray myrequestfromqueryarr = myregvalue.toArray();
            QJsonValue dataFirstarr = myrequestfromqueryarr.first();
            QJsonObject myrequestfromquery = dataFirstarr.toObject();
            // QString queryusername =  myrequestfromquery.value("username").toString();
            QString queryiv =  myrequestfromquery.value("iv").toString();
            QString querycipher =  myrequestfromquery.value("cipher").toString();
            QString theid = myrequestfromquery.value("cryptocenter_id").toString();
            QString mykey = "none";
            qint64 querypolicy =  myrequestfromquery.value("policy").toInt();

            if(querypolicy==0) {

                mykey = myrequestfromquery.value("key").toString();

            }

            else {

                mykey = myrequest.value("key").toString();

            }

            QVariantMap v;
            v.insert("key", mykey);
            v.insert("iv", queryiv);
            v.insert("cipher", querycipher);
            // qDebug() << "iv is: " << queryiv << Qt::endl;
            //  qDebug() << "key is: " << mykey << Qt::endl;
            // qDebug() << "cipher is: " << querycipher << Qt::endl;
            //  qDebug() << "password is: " << mypassword << Qt::endl;

            bool feedback = mypayload->decryptpass(mypassword.toStdString(),v);

            if(feedback) {


                qx::QxRestApi dataapi;
                QRandomGenerator64 *gen = QRandomGenerator64::system();
                gen->securelySeeded();
                long long discardx = gen->bounded(1000000000);
                // qDebug() << "my token  is: " << discardx << Qt::endl;
                QJsonDocument mydoc = QJsonDocument::fromJson(strPayload.toUtf8());
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "query");
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "relations");
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("cryptocenter"));
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "columns[0][0]", "token");
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.token", QJsonValue(discardx));
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.ids_id");
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.password");
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.subscription");
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.username");
                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.cryptocenter_id", QJsonValue(theid));
                QString mycryptojson = mydoc.toJson(QJsonDocument::JsonFormat::Indented);
                //  qDebug() << "my mycryptojson is: " << qPrintable(mycryptojson) << Qt::endl;
                QString resultcrypto = dataapi.processRequest(mycryptojson);
                Q_UNUSED(resultcrypto)
                // qDebug() << "resultcrypto for token in relation cryptocenter is: " << resultcrypto  << Qt::endl;
                QJsonWebToken m_jwtObj;
                QDateTime mycurrent = QDateTime::currentDateTime();
                QString mydate = mycurrent.toString(Qt::ISODate);
                mypayload->processRequest(resultcrypto);
                QJsonObject myregpayload = mypayload->m_requestJson.toObject();
                QJsonValue myregvalue = myregpayload.value("data");
                QJsonObject myregrequest = myregvalue.toObject();
                QString myids_id =  myregrequest.value("cryptocenter_id").toString();
                m_jwtObj.removeAll();
                m_jwtObj.appendClaim(myusername,  QString::number(discardx));
                m_jwtObj.setAlgorithmStr("HSSha3_512");
                m_jwtObj.setSecret(strSecret);
                QJsonArray newarr;
                QJsonObject object;
                object.insert("token", m_jwtObj.getToken());
                object.insert("ids_id", myids_id);
                object.insert("date", QString::fromStdString(mydate.toStdString()));
                newarr << object;
                QJsonDocument responsedoc(newarr);
                //  QString myresponsedocjson = responsedoc.toJson(QJsonDocument::JsonFormat::Indented);
                //   qDebug() << "myresponsedocjson is: " << qPrintable(myresponsedocjson) << Qt::endl;
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);
                errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                         << myrequest.value("created_at").toString().toStdString() << " ." ;
                sccentry << " Logging in of user " << myrequest.value("username").toString().toStdString() <<" at "
                         << myrequest.value("created_at").toString().toStdString() << " was successful." ;





            }

            else {

                const std::string ErrorTemplate = "wrongpassword";
                QString result = (QString::fromStdString(ErrorTemplate));
                response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
                response.status() = 403;
                response.data() = result.toUtf8();
                sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                         << myrequest.value("created_at").toString().toStdString() << " ." ;
                errentry << " Logging in of user " << myrequest.value("username").toString().toStdString() <<" at "
                         << myrequest.value("created_at").toString().toStdString() << " was not successful since a wrong password was supplied." ;


            }


        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " Logging in of user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;



        }




    });


    // End login user



    // Start Retrieve password for a  user

    http.dispatch("POST", "/deliva/ids/hint", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errhint.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS +"/scchint.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString queryparam =  request.params().value("reason");
        // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        //   QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        QString strPayload = QString::fromUtf8(request.data());
        //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myusername =  myrequest.value("username").toString();
        QString myhint =  myrequest.value("hint").toString();

        long long token1 = queryparam.toLongLong();

        QMap<QString, QVariant> params;
        params["username"] = myusername;
        params["token"]    = token1;
        params["hint"]     = myhint;  // if myhint is empty, helper will drop it

        // token is numeric
        QSet<QString> numericKeys;
        numericKeys << "token";

        QString whereClause = buildWhereClause(params, numericKeys);
        // Example result: "username = 'alice' AND token = 12345 AND hint = 'my hint'"
        // (if myhint is empty -> "username = 'alice' AND token = 12345")



        QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
        // qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        //  modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", whereClause);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        // qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        Q_UNUSED(resultcrypto)
        //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        // QString queryusername =  myrequestfromquery.value("username").toString();
        QString queryiv =  myrequestfromquery.value("iv").toString();
        QString querycipher =  myrequestfromquery.value("cipher").toString();
        QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        QString mykey = "none";
        qint64 querypolicy =  myrequestfromquery.value("policy").toInt();

        if(querypolicy==0) {

            mykey = myrequestfromquery.value("key").toString();

        }

        else {

            mykey = myrequest.value("key").toString();

        }

        QVariantMap v;
        v.insert("key", mykey);
        v.insert("iv", queryiv);
        v.insert("cipher", querycipher);
        // qDebug() << "iv is: " << queryiv << Qt::endl;
        //  qDebug() << "key is: " << mykey << Qt::endl;
        // qDebug() << "cipher is: " << querycipher << Qt::endl;

        QString mypasscred = QString::fromStdString(mypayload->recoverpass(v));

        if(!(mypasscred=="")) {



            QJsonArray newarr;
            QJsonObject object;
            object.insert("password", mypasscred);
            object.insert("ids_id", theid);
            newarr << object;
            QJsonDocument responsedoc(newarr);
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = responsedoc.toJson(QJsonDocument::Compact);
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " Retrieving password for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was successful." ;





        }

        else {

            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " Retrieving password for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since"
                                                                                  " access was denied." ;


        }






    });


    // End Retrieve password for a  user




    // Start basic updates

    http.dispatch("POST", "/deliva/universal/basicupdates", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errbasicupdates.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccbasicupdates.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString queryparam =  request.params().value("reason");
        // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        //   QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        QString strPayload = QString::fromUtf8(request.data());
        //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myaction =  mydearpayload.value("action").toString();
        QString myentity =  mydearpayload.value("entity").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myusername =  myrequest.value("username").toString();
        long long token1 = queryparam.toLongLong();

        QMap<QString, QVariant> params;
        params["username"] = myusername;
        params["token"]    = token1;       // numeric

        // Mark numeric fields
        QSet<QString> numericKeys;
        numericKeys << "token";

        QString whereClause = buildWhereClause(params, numericKeys);
        // -> "username = 'bob' AND token = 12345"

        QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
        // qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", whereClause);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        Q_UNUSED(resultcrypto)
        //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
        //  qDebug() << "token1 is: " << token1 << Qt::endl;



        if((querytoken==token1)) {

            QJsonDocument operationdoc = QJsonDocument::fromJson(strPayload.toUtf8());

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="cryptocenter") && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist"  ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("nullandvoid"));

            }

            if(myaction=="update") {

                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "query.sql");
            }

            if(myaction=="fetch_by_id") {

                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "query.sql");
                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.username");

            }

            if(myaction=="fetch_by_query") {

                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data");

            }


            QString operationjson = operationdoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "operationjson is: " << qPrintable(operationjson) << Qt::endl;
            qx::QxRestApi dataapi;
            QString resultdata = dataapi.processRequest(operationjson);
            //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
            /*  qDebug() << "Json Doc for resultdata id_ids is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented))
                  << Qt::endl; */

            if(myentity=="ids") {

                modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "entity", QJsonValue("cryptocenter"));
                modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "data.ids_id");
                modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "data.cryptocenter_id", QJsonValue(theid));
                QString ccjson = operationdoc .toJson(QJsonDocument::JsonFormat::Indented);
                //   qDebug() << "ccjson is: " << qPrintable(ccjson) << Qt::endl;
                QString resultdatacc = dataapi.processRequest(ccjson);
                //  QJsonDocument myccdoc = QJsonDocument::fromJson(resultdatacc.toUtf8());
                /* qDebug() << "Json Doc for resultdatacc is: " << qPrintable(myccdoc.toJson(QJsonDocument::JsonFormat::Indented))
                   << Qt::endl; */
                Q_UNUSED(resultdatacc)

            }

            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = resultdata.toUtf8();
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " basic updates for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was successful." ;








        }

        else {

            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " basic updates for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


        }






    });


    // End basic updates



    // Start password edit

    http.dispatch("POST", "/deliva/ids/secure", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errpasswordupdate.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccpasswordupdate.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString queryparam =  request.params().value("reason");
        // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        //   QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        QString strPayload = QString::fromUtf8(request.data());
        //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myaction =  mydearpayload.value("action").toString();
        QString myentity =  mydearpayload.value("entity").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myusername =  myrequest.value("username").toString();
        QString mypassword =  myrequest.value("password").toString();
        long long token1 = queryparam.toLongLong();

        QMap<QString, QVariant> params;
        params["username"] = myusername;
        params["token"]    = token1;       // numeric

        // Mark numeric fields
        QSet<QString> numericKeys;
        numericKeys << "token";

        QString whereClause = buildWhereClause(params, numericKeys);
        // -> "username = 'bob' AND token = 12345"

        QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
        // qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", whereClause);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        Q_UNUSED(resultcrypto)
        //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        int myrole = myrequestfromquery.value("roles").toInt();
        qint64 mypolicy = myrequestfromquery.value("policy").toString().toInt();



        if((querytoken==token1)) {

            QJsonDocument operationdoc = QJsonDocument::fromJson(strPayload.toUtf8());

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("nullandvoid"));

            }


            modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "query");
            //  QString operationjson = operationdoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "operationjson is: " << qPrintable(operationjson) << Qt::endl;
            qx::QxRestApi dataapi;
            QVariantMap mysecurity = modifyMyJsonToolsuser->encryptpass(mypassword.toStdString());
            QString mycryptokey = mysecurity.value("key").toString();
            modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("cryptocenter"));
            modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "query.entity", QJsonValue("cryptocenter"));

            if(mypolicy==0) {


                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.key", QJsonValue(mycryptokey));

            }

            else {


                modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.key", QJsonValue("policyretain"));


            }

            QStringList m_destlist;
            m_destlist << "iv";
            m_destlist << "key";
            m_destlist << "cipher";
            QJsonArray m_array = QJsonArray::fromStringList(m_destlist);
            //   qDebug() << "m_array is: " << m_array << Qt::endl;
            modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "columns");
            modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "columns", m_array);
            modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.iv",  QJsonValue(mysecurity.value("iv").toString()));
            modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.cipher",  QJsonValue(mysecurity.value("cipher").toString()));
            modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "data.ids_id");
            modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "data.cryptocenter_id", QJsonValue(theid));
            QString ccjson = operationdoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "ccjson is: " << qPrintable(ccjson) << Qt::endl;
            QString resultdatacc = dataapi.processRequest(ccjson);
            //  QJsonDocument myccdoc = QJsonDocument::fromJson(resultdatacc.toUtf8());
            /* qDebug() << "Json Doc for resultdatacc is: " << qPrintable(myccdoc.toJson(QJsonDocument::JsonFormat::Indented))
                   << Qt::endl; */
            Q_UNUSED(resultdatacc)
            mypayload->processRequest(resultdatacc);
            QJsonObject myregpayload = mypayload->m_requestJson.toObject();
            QJsonValue myregvalue = myregpayload.value("data");
            QJsonObject myregrequest = myregvalue.toObject();
            QString myids_id =  myregrequest.value("cryptocenter_id").toString();
            QJsonArray newarr;
            QJsonObject object;

            if(mypolicy==1) {

                object.insert("key", mycryptokey);

            }



            object.insert("ids_id", myids_id);
            newarr << object;
            QJsonObject obj;
            obj [ "data" ] = newarr;
            QJsonDocument responsedoc(obj);
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = responsedoc.toJson(QJsonDocument::Compact);
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " password edit for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString()<< " was successful." ;







        }

        else {

            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " password edit for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


        }






    });


    // End password edit





    // Start get user details


    http.dispatch("POST", "/deliva/ids/user", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        //  QString queryparam =  request.params().value("reason");
        //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        // QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/erruserdetails.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccuserdetails.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString strPayload = QString::fromUtf8(request.data());
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myaction =  mydearpayload.value("action").toString();
        QString myentity =  mydearpayload.value("entity").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString mytoken =  request.params().value("reason");
        QString myusername =  myrequest.value("username").toString();
        // qDebug() << "mytoken is: " << mytoken << Qt::endl;
        long long token1 = mytoken.toLongLong();

        QMap<QString, QVariant> params;
        params["username"] = myusername;
        params["token"]    = token1;       // numeric

        // Mark numeric fields
        QSet<QString> numericKeys;
        numericKeys << "token";

        QString whereClause = buildWhereClause(params, numericKeys);
        // -> "username = 'bob' AND token = 12345"
        QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", whereClause);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //qDebug() << "myfinaljson after modification is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        // QString queryresults =  myrequestfromquery.value("username").toString();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
        //  qDebug() << "token1 is: " << token1 << Qt::endl;



        if((querytoken==token1)) {



            qx::QxRestApi dataapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "myusagejson in ids is: " << qPrintable(myusagejson) << Qt::endl;
            QString resultdata = dataapi.processRequest(myusagejson);
            //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
            //  qDebug() << "Json Doc for resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
            mypayload->processRequest(resultdata);
            QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
            QJsonValue myregdata = mydearpayloaddata.value("data");
            QJsonArray myqueryarr = myregdata.toArray();
            QJsonDocument responsedoc(myqueryarr);
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = responsedoc.toJson(QJsonDocument::Compact);
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " fetch details for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " fetch details for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


        }



    });


    // End get user details


    // Start update subscription


    http.dispatch("POST", "/deliva/ids/subscribe", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errsubscribe.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccsubscribe.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString queryparam =  request.params().value("reason");
        // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        QString strPayload = QString::fromUtf8(request.data());
        QString mytoken = queryparam;
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myaction =  mydearpayload.value("action").toString();
        QString myentity =  mydearpayload.value("entity").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myauth =  myrequest.value("auth").toString();
        Q_UNUSED(myauth)
        QString myusername =  myrequest.value("username").toString();
        QString myidids = myrequest.value("ids_id").toString();
        qint64 mysub =  myrequest.value("subscription").toInt();
        // qDebug() << "myauth is: " << myauth << Qt::endl;
        qDebug() << "myusername is: " << myusername << Qt::endl;
        // qDebug() << "mysub is: " << mysub << Qt::endl;
        // qDebug() << "mytoken is: " << mytoken << Qt::endl;
        long long token1 = mytoken.toLongLong();

        QMap<QString, QVariant> params;
        params["username"] = myusername;
        params["token"]    = token1;       // numeric

        // Mark numeric fields
        QSet<QString> numericKeys;
        numericKeys << "token";

        QString whereClause = buildWhereClause(params, numericKeys);
        // -> "username = 'bob' AND token = 12345"
        QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", whereClause);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        qDebug() << "myfinaljson in subscription update is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        Q_UNUSED(resultcrypto)
        //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        //QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
        //  qDebug() << "token1 is: " << token1 << Qt::endl;



        if((querytoken==token1)) {

            qx::QxRestApi dataapi;

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("nullandvoid"));

            }

            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "query");
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.auth");
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.username");
            QString mydocjson = mydoc.toJson(QJsonDocument::JsonFormat::Indented);
            // qDebug() << "my mydocjson  is: " << qPrintable(mydocjson) << Qt::endl;
            QString resultdata = dataapi.processRequest(mydocjson);
            Q_UNUSED(resultdata)
            //  qDebug() << "resultdata for subscription in relation ids is: " << resultdata  << Qt::endl;
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("cryptocenter"));
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.ids_id");
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.cryptocenter_id", QJsonValue(myidids));
            QString mycryptojson = mydoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "my mycryptojson is: " << qPrintable(mycryptojson) << Qt::endl;
            QString resultcrypto = dataapi.processRequest(mycryptojson);
            Q_UNUSED(resultcrypto)
            //   qDebug() << "resultcrypto for subscription in relation cryptocenter is: " << resultcrypto  << Qt::endl;
            const std::string ErrorTemplate = "success";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 200;
            response.data() = result.toUtf8();
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " subscription update for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString()<< " was successful." ;



        }

        else {


            const std::string SuccTemplate = "denied";
            QString result = (QString::fromStdString(SuccTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " subscription update for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString()<< " was not successful since access was denied." ;


        }




    });

    // End update subscription


    // Start signout user

    http.dispatch("POST", "/deliva/ids/logout", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errlogout.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/scclogout.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString queryparam =  request.params().value("reason");
        // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        QString strPayload = QString::fromUtf8(request.data());
        QString mytoken = queryparam;
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myaction =  mydearpayload.value("action").toString();
        QString myentity =  mydearpayload.value("entity").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myauth =  myrequest.value("auth").toString();
        Q_UNUSED(myauth)
        QString myusername =  myrequest.value("username").toString();
        QString myidids = myrequest.value("ids_id").toString();
        qint64 mysub =  myrequest.value("subscription").toInt();
        // qDebug() << "myauth is: " << myauth << Qt::endl;
        // qDebug() << "myusername is: " << myusername << Qt::endl;
        // qDebug() << "mysub is: " << mysub << Qt::endl;
        // qDebug() << "mytoken is: " << mytoken << Qt::endl;
        long long token1 = mytoken.toLongLong();

        QMap<QString, QVariant> params;
        params["username"] = myusername;
        params["token"]    = token1;       // numeric

        // Mark numeric fields
        QSet<QString> numericKeys;
        numericKeys << "token";

        QString whereClause = buildWhereClause(params, numericKeys);
        qInfo()<< "where clause is: " << whereClause << Qt::endl;
        // -> "username = 'bob' AND token = 12345"
        QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", whereClause);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        // qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        Q_UNUSED(resultcrypto)
        //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //qInfo() << "querytoken is: " << querytoken << Qt::endl;
        //qInfo() << "token1 is: " << token1 << Qt::endl;



        if((querytoken==token1)) {

            qx::QxRestApi dataapi;
            QRandomGenerator64 *gen = QRandomGenerator64::system();
            gen->securelySeeded();
            long long discardx = gen->bounded(1000000000);
          //  qInfo() << "my token  is: " << discardx << Qt::endl;

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="ids" || myentity=="cryptocenter"  ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist"  ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("nullandvoid"));

            }


            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.logout");
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "query");
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("cryptocenter"));
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "columns[0][0]", "token");
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.token", QJsonValue(discardx));
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.ids_id");
            modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.cryptocenter_id", QJsonValue(myidids));
            QString mycryptojson = mydoc.toJson(QJsonDocument::JsonFormat::Indented);
            //   qDebug() << "my mycryptojson is: " << qPrintable(mycryptojson) << Qt::endl;
            QString resultcrypto = dataapi.processRequest(mycryptojson);
            Q_UNUSED(resultcrypto)
            // qDebug() << "resultcrypto for token in relation cryptocenter is: " << resultcrypto  << Qt::endl;
            const std::string TokenTemplate = QString::number(discardx).toStdString();
            QString result = (QString::fromStdString(TokenTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 200;
            response.data() = result.toUtf8();
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " logout operation for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was successful." ;



        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " logout operation for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


        }


    });


    // End signout user



    // Start create new category

    http.dispatch("POST", "/deliva/category/new", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errnewcategory.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccnewcategory.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        // QJsonDocument originaldoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "originaldoc is: " << qPrintable(originaldoc.toJson(QJsonDocument::Indented)) << Qt::endl;
        // QString myrequestdoc = QString::fromUtf8(request.data());
        QString queryparam =  request.params().value("reason");
        // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        QString strPayload = QString::fromUtf8(request.data());
        QString mytoken = queryparam;
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myaction =  mydearpayload.value("action").toString();
        QString myentity =  mydearpayload.value("entity").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myusername =  myrequest.value("username").toString();
        //   QString myidids = myrequest.value("ids_id").toString();
        qint64 mysub =  myrequest.value("subscription").toInt();
        // qDebug() << "myidids is: " << myidids << Qt::endl;
        // qDebug() << "myusername is: " << myusername << Qt::endl;
        // qDebug() << "mysub is: " << mysub << Qt::endl;
        // qDebug() << "mytoken is: " << mytoken << Qt::endl;
        long long token1 = mytoken.toLongLong();

        QMap<QString, QVariant> params1;
        params1["username"] = myusername;
        params1["token"]    = token1;

        QSet<QString> numericKeys;
        numericKeys << "token";

        QString where1 = buildWhereClause(params1, numericKeys);
        // Example: "username = 'bob' AND token = 12345"

        QMap<QString, QVariant> params2;
        params2["name"]     = myrequest.value("name").toString();
        params2["username"] = myusername;

        QString where2 =buildWhereClause(params2,QSet<QString>(),"categories");
        // Example: "name = 'John Doe' AND username = 'bob'"

        QJsonValue myvaluequery = mydearpayload.value("query");
        QJsonObject myrequestquery = myvaluequery.toObject();
        //  QString myentityquery =  myrequestquery.value("entity").toString();
        // qDebug() << "myentityquery is: " << myentityquery << Qt::endl;
        QJsonDocument userdoc(myrequestquery);
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where1);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "myfinaljson in category crypto is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        Q_UNUSED(resultcrypto)
        // qDebug() << "result after query in crypto category is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        //  QString queryresults =  myrequestfromquery.value("username").toString();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
        //  qDebug() << "token1 is: " << token1 << Qt::end
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where2);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("categories"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        QString mycatjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //   qDebug() << "mycatjson in categories is: " << qPrintable(mycatjson) << Qt::endl;
        qx::QxRestApi catapi;
        QString resultcat = catapi.processRequest(mycatjson);
        //   qDebug() << "result after query in categories is: " << resultcat << Qt::endl;
        mypayload->processRequest(resultcat);
        QJsonObject mydearpayloadcatresults = mypayload->m_requestJson.toObject();
        QJsonValue mycatvalue = mydearpayloadcatresults.value("data");
        QJsonArray myrequestcatfromqueryarr = mycatvalue.toArray();
        QJsonValue datacatFirstarr = myrequestcatfromqueryarr.first();
        QJsonObject myrequestcatfromquery = datacatFirstarr.toObject();
        QString querycatresults =  myrequestcatfromquery.value("name").toString();
        //   qDebug() << "querycatresults in categories is: " << querycatresults << Qt::endl;

        if(querytoken==token1 && !(querycatresults==myrequest.value("name").toString())) {




            qx::QxRestApi loadapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"   || myaction =="update" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }


            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query");
            QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "myusagejson in category is: " << qPrintable(myusagejson) << Qt::endl;
            QString resultreg = loadapi.processRequest(myusagejson);
            // qDebug() << "result after inserting new category is: " << resultreg << Qt::endl;
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = resultreg.toUtf8();
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " creation of new category for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " creation of new category for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;



        }



    });

    // End create new category


    // Start create new item

    http.dispatch("POST", "/deliva/menu/new", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/erritemnew.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccitemnew.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QByteArray utfString = request.data();
        const long mysize  =  request.header("Content-Length").toLong();
        std::string multipartContent = request.header("Content-Type").toStdString();
        std::string jsondata ="";
        jsondata = mypayload->doupload(utfString,mypayload->UPLOAD_TEMPDIRIDS,
                                       mypayload->UPLOAD_DIRIDS,mysize,multipartContent,false);

        //   QJsonDocument originaldoc = QJsonDocument::fromJson(QString::fromStdString(jsondata).toUtf8());
        //  qDebug() << "originaldoc is: " << qPrintable(originaldoc.toJson(QJsonDocument::Indented)) << Qt::endl;
        // QString myrequestdoc = QString::fromUtf8(QString::fromStdString(jsondata).toUtf8());
        QString queryparam =  request.params().value("reason");
        // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        QString strPayload = QString::fromStdString(jsondata);
        QString mytoken = queryparam;
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myentity =  mydearpayload.value("entity").toString();
        QString myaction =  mydearpayload.value("action").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myusername =  myrequest.value("username").toString();
        long long token1 = mytoken.toLongLong();

        QMap<QString, QVariant> params1;
        params1["username"] = myusername;
        params1["token"]    = token1;

        QSet<QString> numericKeys;
        numericKeys << "token";

        QString where1 = buildWhereClause(params1, numericKeys);
        // Example: "username = 'bob' AND token = 12345"

        QMap<QString, QVariant> params2;
        params2["name"]     = myrequest.value("name").toString();
        params2["username"] = myusername;


        QString where2 =buildWhereClause(params2,QSet<QString>(),"items");



        // Example: "name = 'John Doe' AND username = 'bob'"

        QJsonValue myvaluequery = mydearpayload.value("query");
        QJsonObject myrequestquery = myvaluequery.toObject();
        //  QString myentityquery =  myrequestquery.value("entity").toString();
        // qDebug() << "myentityquery is: " << myentityquery << Qt::endl;
        QJsonDocument userdoc(myrequestquery);
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where1);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        // qDebug() << "myfinaljson in items crypto is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        Q_UNUSED(resultcrypto)
        //  qDebug() << "result after query in crypto items is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        // QString queryresults =  myrequestfromquery.value("username").toString();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        // QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
        //  qDebug() << "token1 is: " << token1 << Qt::endl;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where2);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("items"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        QString myitemsjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "myitemsjson in items is: " << qPrintable(myitemsjson) << Qt::endl;
        qx::QxRestApi itemsapi;
        QString resultitems = itemsapi.processRequest(myitemsjson);
        //  qDebug() << "result after query in items is: " << resultitems << Qt::endl;
        mypayload->processRequest(resultitems);
        QJsonObject mydearpayloaditemsresults = mypayload->m_requestJson.toObject();
        QJsonValue myitemsvalue = mydearpayloaditemsresults.value("data");
        QJsonArray myrequestitemsfromqueryarr = myitemsvalue.toArray();
        QJsonValue dataitemsFirstarr = myrequestitemsfromqueryarr.first();
        QJsonObject myrequestitemsfromquery = dataitemsFirstarr.toObject();
        QString queryitemsresults =  myrequestitemsfromquery.value("name").toString();
        //  qDebug() << "queryitemsresults in items is: " << queryitemsresults << Qt::endl;

        if(querytoken==token1 && !(queryitemsresults==myrequest.value("name").toString())) {


            jsondata = mypayload->doupload(utfString,mypayload->UPLOAD_TEMPDIRIDS,
                                           mypayload->UPLOAD_DIRIDS,mysize,multipartContent,true,false,"");


            qx::QxRestApi loadapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(QString::fromStdString(jsondata).toUtf8());

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="update" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }


            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query");
           // modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query.sql", where2); // might not be usweful here
            QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "myusagejson in item is: " << qPrintable(myusagejson) << Qt::endl;
            QString resultitemz = loadapi.processRequest(myusagejson);
            //  qDebug() << "result after inserting new item is: " << resultitemz<< Qt::endl;
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = resultitemz.toUtf8();
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " creation of new item for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was successful." ;





        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " creation of new item for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


        }



    });

    // End create new item

    // Start fetch specific

    http.dispatch("POST", "/deliva/universal/fetchspecific", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        //  QString queryparam =  request.params().value("reason");
        //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        //  QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchspecific.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchspecific.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString strPayload = QString::fromUtf8(request.data());
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myentity =  mydearpayload.value("entity").toString();
        QString myaction =  mydearpayload.value("action").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString mytoken =  request.params().value("reason");
        QString myusername =  myrequest.value("username").toString();
        QString mytarget =  mydearpayload.value("target").toString();
        //  qDebug() << "mytarget is: " << mytarget << Qt::endl;
        // qDebug() << "mytoken is: " << mytoken << Qt::endl;
        long long token1 = mytoken.toLongLong();

        QMap<QString, QVariant> params;
        params["username"] = myusername;
        params["token"]    = token1;       // numeric

        // Mark numeric fields
        QSet<QString> numericKeys;
        numericKeys << "token";

        QString whereClause = buildWhereClause(params, numericKeys);
        // -> "username = 'bob' AND token = 12345"
        QString whereids =buildWhereClause(params,QSet<QString>(),"ids");

        QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", whereClause);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //qDebug() << "myfinaljson after modification is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
        //  qDebug() << "token1 is: " << token1 << Qt::endl;



        if((querytoken==token1)) {

            qx::QxRestApi dataapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());
            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query.sql", whereids); // added
           // modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("ids")); // added

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }


            QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
            //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
            //  qDebug() << "Json Doc for resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
            mypayload->processRequest(resultdata);
            QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
            QJsonValue myregdata = mydearpayloaddata.value("data");
            QJsonArray myqueryarr = myregdata.toArray();
            QJsonValue dataFirstarr = myqueryarr.first();
            QJsonObject myrequestfromquery = dataFirstarr.toObject();

            if(mytarget == "categories") {

                QJsonValue myregdatacat = myrequestfromquery.value("list_catids");
                QJsonArray myqueryarrcat = myregdatacat.toArray();
                QJsonDocument responsedoc(myqueryarrcat);
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);

            }

            else if (mytarget == "items") {


                QJsonValue myregdataitem = myrequestfromquery.value("list_itemsids");
                QJsonArray myqueryarritem = myregdataitem.toArray();
                QJsonDocument responsedoc(myqueryarritem);
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);


            }

            else if (mytarget == "delivery") {

                QJsonValue myregdatadeli = myrequestfromquery.value("list_deliveryids");
                QJsonArray myqueryarrdeli = myregdatadeli.toArray();
                QJsonDocument responsedoc(myqueryarrdeli);
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);



            }

            else if (mytarget == "logistics") {

                QJsonValue myregdatalogi = myrequestfromquery.value("list_logisticsids");
                QJsonArray myqueryarrlogi = myregdatalogi.toArray();
                QJsonDocument responsedoc(myqueryarrlogi);
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);



            }

            else if (mytarget == "orders") {


                QJsonValue myregdataord = myrequestfromquery.value("list_ordersids");
                QJsonArray myqueryarrord = myregdataord.toArray();
                QJsonDocument responsedoc(myqueryarrord);
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);


            }



            else if (mytarget == "locations") {

                QJsonValue myregdataloc = myrequestfromquery.value("list_locationsids");
                QJsonArray myqueryarrloc= myregdataloc.toArray();
                QJsonDocument responsedoc(myqueryarrloc);
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);


            }

            else if (mytarget == "metrics") {


                QJsonValue myregdatamet = myrequestfromquery.value("list_metricsids");
                QJsonArray myqueryarrmet= myregdatamet.toArray();
                QJsonDocument responsedoc(myqueryarrmet);
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);



            }

            else {


                QJsonDocument responsedoc(myqueryarr);
                response.headers().insert("Content-Type", "application/json; charset=utf-8");
                response.data() = responsedoc.toJson(QJsonDocument::Compact);


            }




            // QString queryresults =  myrequestfromquery.value("username").toString();
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " fetch specific data for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was successful." ;





        }

        else {

            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " fetch specific data for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;





        }


    });


    // End fetch specific


    // Start update basic avatars

    http.dispatch("POST", "/deliva/universal/updatebasicavatars", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errupdatebasicavatars.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccupdatebasicavatars.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QByteArray utfString = request.data();
        const long mysize  =  request.header("Content-Length").toLong();
        std::string multipartContent = request.header("Content-Type").toStdString();
        std::string jsondata ="";
        jsondata = mypayload->doupload(utfString,mypayload->UPLOAD_TEMPDIRIDS,
                                       mypayload->UPLOAD_DIRIDS,mysize,multipartContent,false);
        //  QJsonDocument originaldoc = QJsonDocument::fromJson(QString::fromStdString(jsondata).toUtf8());
        //  qDebug() << "originaldoc is: " << qPrintable(originaldoc.toJson(QJsonDocument::Indented)) << Qt::endl;
        // QString myrequestdoc = QString::fromUtf8(QString::fromStdString(jsondata).toUtf8());
        QString queryparam =  request.params().value("reason");
        // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        QString strPayload = QString::fromStdString(jsondata);
        QString mytoken = queryparam;
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myfield =  mydearpayload.value("field").toString();
        QString myavatarfield =  mydearpayload.value("avatarfield").toString();
        QString myentity =  mydearpayload.value("entity").toString();
        QString myaction =  mydearpayload.value("action").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myusername =  myrequest.value("username").toString();
        // assume buildWhereClause(...) and escapeSqlString(...) exist in a util file

        long long token1 = mytoken.toLongLong();

        // Build params for the token+username check (query1)
        QMap<QString, QVariant> params1;
        params1["username"] = myusername;
        params1["token"]    = token1;

        QSet<QString> numericKeys;
        numericKeys << "token"; // token is numeric

        QString where1 = buildWhereClause(params1, numericKeys);
        // e.g. "username = 'bob' AND token = 12345"

        // Build params for the dynamic field check (query2)
        QMap<QString, QVariant> params2;
        QVariant fieldVal = myrequest.value(myfield);

        // Only insert if value is valid (buildWhereClause will skip empty strings and nulls,
        // but we can still guard here if desired)
        if (fieldVal.isValid() && !fieldVal.isNull()) {
            params2.insert(myfield, fieldVal);
        }

        QString where2 =buildWhereClause(params2,QSet<QString>(),myentity);

        // e.g. "email = 'joe@example.com'"  (or "" if fieldVal was empty -> you can handle that)

        QJsonValue myvaluequery = mydearpayload.value("query");
        QJsonObject myrequestquery = myvaluequery.toObject();
        //  QString myentityquery =  myrequestquery.value("entity").toString();
        // qDebug() << "myentityquery is: " << myentityquery << Qt::endl;
        QJsonDocument userdoc(myrequestquery);
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where1);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        // qDebug() << "myfinaljson in update basic avatars crypto is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        Q_UNUSED(resultcrypto)
        //  qDebug() << "result after query in crypto update basic avatars  is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        // QString queryresults =  myrequestfromquery.value("username").toString();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        // QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
        //  qDebug() << "token1 is: " << token1 << Qt::endl;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where2);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue(myentity));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        QString myitemsjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "myitemsjson in update basic avatars  is: " << qPrintable(myitemsjson) << Qt::endl;
        qx::QxRestApi itemsapi;
        QString resultitems = itemsapi.processRequest(myitemsjson);
        //  qDebug() << "result after query in update basic avatars  is: " << resultbasicavatars << Qt::endl;
        mypayload->processRequest(resultitems);
        QJsonObject mydearpayloaditemsresults = mypayload->m_requestJson.toObject();
        QJsonValue myitemsvalue = mydearpayloaditemsresults.value("data");
        QJsonArray myrequestitemsfromqueryarr = myitemsvalue.toArray();
        QJsonValue dataitemsFirstarr = myrequestitemsfromqueryarr.first();
        QJsonObject myrequestitemsfromquery = dataitemsFirstarr.toObject();
        QString queryitemsresults =  myrequestitemsfromquery.value(myfield).toString();
        QString queryavatarnameresults =  myrequestitemsfromquery.value(myavatarfield).toString();
        //  qDebug() << "queryitemsresults in update basic avatars  is: " << queryitemsresults << Qt::endl;

        if(querytoken==token1 && (queryitemsresults==myrequest.value(myfield).toString())) {

            jsondata = mypayload->doupload(utfString,mypayload->UPLOAD_TEMPDIRIDS,
                                           mypayload->UPLOAD_DIRIDS,mysize,multipartContent,true,true,
                                           queryavatarnameresults.toStdString());
            qx::QxRestApi loadapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(QString::fromStdString(jsondata).toUtf8());

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }


            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query");
            QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "myusagejson in update basic avatars  is: " << qPrintable(myusagejson) << Qt::endl;
            QString resultitemz = loadapi.processRequest(myusagejson);
            //  qDebug() << "result after update basic avatars  is: " << resultitemz<< Qt::endl;
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = resultitemz.toUtf8();
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " updating basic avatars for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was successful." ;





        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " updating basic avatars for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


        }


    });

    // End update basic avatars


    // Start other operations

    http.dispatch("POST", "/deliva/universal/otheroperations", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errotheroperations.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccotheroperations.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString strPayload = QString::fromUtf8(request.data());
        //  QJsonDocument originaldoc = QJsonDocument::fromJson(strPayload.toUtf8());
        //  qDebug() << "originaldoc is: " << qPrintable(originaldoc.toJson(QJsonDocument::Indented)) << Qt::endl;
        //  QString myrequestdoc = QString::fromUtf8(strPayload.toUtf8());
        QString queryparam =  request.params().value("reason");
        // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        QString mytoken = queryparam;
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myfield =  mydearpayload.value("field").toString();
        QString myavatarfield =  mydearpayload.value("avatarfield").toString();
        //  qDebug() <<"avatar field is: " << myavatarfield  << Qt::endl;
        /*   if(myavatarfield=="" || myavatarfield.isEmpty() || myavatarfield.isNull()) {

           myavatarfield="nullstr";

        } */

        if(myavatarfield=="") {

            myavatarfield="nullstr";

        }

        else {

            qInfo() << "avatar is: " << myavatarfield << Qt::endl;
        }

        bool deleteother =  mydearpayload.value("deleteother").toBool();
        QString deleteotherfield =  mydearpayload.value("deleteotherfield").toString();
        QString deleteotherentity =  mydearpayload.value("deleteotherentity").toString();
        QString myentity =  mydearpayload.value("entity").toString();
        QString myaction =  mydearpayload.value("action").toString();
        QJsonValue myvalue = mydearpayload.value("info");
        QJsonObject myrequest = myvalue.toObject();
        QString myusername =  myrequest.value("username").toString();
        // --- assume these exist in your util file ---
        // QString buildWhereClause(const QMap<QString,QVariant>& params, const QSet<QString>& numericKeys = QSet<QString>());
        // QString escapeSqlString(const QString& s);

        // convert token
        long long token1 = mytoken.toLongLong();


        // --- Build query1: username + token ---
        QMap<QString, QVariant> params1;
        params1["username"] = myusername;
        params1["token"]    = token1;

        QSet<QString> numericKeys;
        numericKeys << "token";

        QString where1 = buildWhereClause(params1, numericKeys);
        // Example: "username = 'bob' AND token = 12345"

        // --- Build query2: dynamic field (myfield) ---
        QString dynField = myfield; // column name (trusted ideally)
        QVariant dynVal = myrequest.value(myfield);

        QString where2;
        if (!dynField.isEmpty() && dynVal.isValid() && !dynVal.isNull()) {
            if (isAllowedColumn(dynField)) {
                QMap<QString, QVariant> params2;
                params2.insert(dynField, dynVal);
                where2 = buildWhereClause(params2,QSet<QString>(),"items");
                // e.g. "email = 'joe@example.com'"
            } else {
                // handle invalid column name (skip or log)
                // e.g. where2 = ""; // skip applying query
            }
        } else {
            // dynVal empty or invalid -> where2 remains empty (builder would skip it anyway)
        }

        // --- Build query3: another dynamic field (otherfield/deleteotherfield) ---
        QString otherCol = mydearpayload.value("otherfield").toString(); // assuming this holds column name
        QVariant otherVal = myrequest.value(deleteotherfield); // value to delete/match

        QString where3;
        if (!otherCol.isEmpty() && otherVal.isValid() && !otherVal.isNull()) {
            if (isAllowedColumn(otherCol)) {
                QMap<QString, QVariant> params3;
                params3.insert(otherCol, otherVal);
                where3 = buildWhereClause(params3,QSet<QString>(),"categories");
            } else {
                // invalid column name; skip or handle error
            }
        } else {
            // otherVal empty or invalid -> where3 empty
        }

        QJsonValue myvaluequery = mydearpayload.value("query");
        QJsonObject myrequestquery = myvaluequery.toObject();
        //  QString myentityquery =  myrequestquery.value("entity").toString();
        // qDebug() << "myentityquery is: " << myentityquery << Qt::endl;
        QJsonDocument userdoc(myrequestquery);
        QJsonDocument myentityfunction = userdoc;
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where1);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        // qDebug() << "myfinaljson in other operations crypto is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        Q_UNUSED(resultcrypto)
        //  qDebug() << "result after query in crypto other operations  is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        // QString queryresults =  myrequestfromquery.value("username").toString();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        // QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
        //  qDebug() << "token1 is: " << token1 << Qt::endl;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where2);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue(myentity));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        QString myitemsjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        // qDebug() << "myitemsjson in other operations  is: " << qPrintable(myitemsjson) << Qt::endl;
        qx::QxRestApi itemsapi;
        QString resultitems = itemsapi.processRequest(myitemsjson);
        //  qDebug() << "result after query in other operations  is: " << resultitems << Qt::endl;
        mypayload->processRequest(resultitems);
        QJsonObject mydearpayloaditemsresults = mypayload->m_requestJson.toObject();
        QJsonValue myitemsvalue = mydearpayloaditemsresults.value("data");
        QJsonArray myrequestitemsfromqueryarr = myitemsvalue.toArray();
        QJsonValue dataitemsFirstarr = myrequestitemsfromqueryarr.first();
        QJsonObject myrequestitemsfromquery = dataitemsFirstarr.toObject();
        QString queryitemsresults =  myrequestitemsfromquery.value(myfield).toString();
        QString queryitemsavatarname =  myrequestitemsfromquery.value(myavatarfield).toString();
        QStringList m_deletemedia;
        m_deletemedia << queryitemsavatarname;
        qx::QxRestApi entityapi;
        QJsonArray m_array = QJsonArray::fromStringList(m_deletemedia);
        modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "action", QJsonValue("call_entity_function"));
        modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "query");
        modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "entity", QJsonValue(myentity));
        modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "fct","deletemediaite");
        modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "data.media", m_array);
        QString myentityjson = myentityfunction.toJson(QJsonDocument::JsonFormat::Indented);
        // qDebug() << "myentityjson in other operations  is: " << qPrintable(myentityjson) << Qt::endl;
        QString entityfctresults = entityapi.processRequest(myentityjson);
        Q_UNUSED(entityfctresults)
        // qDebug() << "result after entityfctresults in other operations  is: " << entityfctresults << Qt::endl;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where3);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue(deleteotherentity));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        QString myanotherjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "myanotherjson in other operations  is: " << qPrintable(myanotherjson) << Qt::endl;
        QString resultanother = itemsapi.processRequest(myanotherjson);
        mypayload->processRequest(resultanother);
        QJsonObject mydearpayloadresultanother = mypayload->m_requestJson.toObject();
        QJsonValue myresultanothervalue = mydearpayloadresultanother.value("data");
        QJsonArray myrequestresultanotherfromqueryarr = myresultanothervalue.toArray();
        QJsonValue dataresultanotherFirstarr = myrequestresultanotherfromqueryarr.first();
        QJsonObject myrequestresultanotherfromquery = dataresultanotherFirstarr.toObject();
        QString queryresultanotherresults =  myrequestresultanotherfromquery.value(deleteotherentity+"_id").toString();

        if(querytoken==token1 && (queryitemsresults==myrequest.value(myfield).toString())) {

            qx::QxRestApi loadapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(strPayload.toUtf8());

            // if((myentity=="ids" || myentity=="cryptocenter"  ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myaction=="call_entity_function"  || myentity=="cryptocenter") && (myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }


            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query");
            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "info");
            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "deleteother");
            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "deleteotherentity");
            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "deleteotherfield");
            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "field");
            QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
            //  qDebug() << "myusagejson in other operations  is: " << qPrintable(myusagejson) << Qt::endl;
            QString resultitemz = loadapi.processRequest(myusagejson);
            //  qDebug() << "result after other operations  is: " << resultitemz<< Qt::endl;
            if(deleteother) {


                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "data."+deleteotherentity+"_id",queryresultanotherresults);
                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "data."+myentity+"_id");
                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue(deleteotherentity));
                QString myusagedeleteotherjson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
                // qDebug() << "myusagedeleteotherjson in other operations  is: " << qPrintable(myusagedeleteotherjson) << Qt::endl;
                QString resultitemzdeleteother = loadapi.processRequest(myusagedeleteotherjson);
                Q_UNUSED(resultitemzdeleteother)
                //  qDebug() << "resultdeleteother after other operations  is: " << resultitemzdeleteother<< Qt::endl;

            }
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = resultitemz.toUtf8();
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " other operations for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was successful." ;





        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " other operations for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


        }


    });

    // End other operations

    // Start fetch all details


    http.dispatch("POST", "/deliva/generic/fetchall", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        //  QString queryparam =  request.params().value("reason");
        //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        // QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc fetch all is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchall.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchall.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString strPayload = QString::fromUtf8(request.data());
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myentity =  mydearpayload.value("entity").toString();
        QString myaction =  mydearpayload.value("action").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString mytoken =  request.params().value("reason");
        QString myusername =  myrequest.value("username").toString();
        // debug: // qDebug() << "mytoken is: " << mytoken << Qt::endl;
        long long token1 = mytoken.toLongLong();

        // Build params for username + token (query1)
        QMap<QString, QVariant> params1;
        params1["username"] = myusername;
        params1["token"]    = token1;

        QSet<QString> numericKeys;
        numericKeys << "token";

        QString where1 = buildWhereClause(params1, numericKeys);
        // where1 -> "username = 'bob' AND token = 12345"

        // Build params for username-only (query0)
        QMap<QString, QVariant> params0;
        params0["username"] = myusername;

        QString where0 = buildWhereClause(params0);
        // where0 -> "username = 'bob'"

       // QString whereids =buildWhereClause(params0,QSet<QString>(),"ids");


        QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where1);
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
        QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //qDebug() << "myfinaljson after modification is: " << qPrintable(myfinaljson) << Qt::endl;
        qx::QxRestApi cryptoapi;
        QString resultcrypto = cryptoapi.processRequest(myfinaljson);
        //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
        mypayload->processRequest(resultcrypto);
        QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
        QJsonValue myregvalue = mydearpayloadresults.value("data");
        QJsonArray myrequestfromqueryarr = myregvalue.toArray();
        QJsonValue dataFirstarr = myrequestfromqueryarr.first();
        QJsonObject myrequestfromquery = dataFirstarr.toObject();
        // QString queryresults =  myrequestfromquery.value("username").toString();
        long long querytoken =  myrequestfromquery.value("token").toInt();
        int myrole = myrequestfromquery.value("roles").toInt();
        //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
        //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
        //  qDebug() << "token1 is: " << token1 << Qt::endl;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where0);
        QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
        qx::QxRestApi cryptoapiadsupwebjson;
        QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
        // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
        mypayload->processRequest(resultcryptoadsupwebjson);
        QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
        QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
        QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
        QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
        QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
        // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
        //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
        qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
        //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





        if((querytoken==token1)) {



            qx::QxRestApi dataapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());
         //   modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query.sql", whereids); // added
          //  modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("ids")); // added




            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }



            // qDebug() << "Usage JSON for fetch all details is: " << qPrintable(usagedoc.toJson(QJsonDocument::Indented)) << Qt::endl;
            QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
            //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
            //  qDebug() << "Json Doc for fetchall resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
            mypayload->processRequest(resultdata);
            QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
            QJsonValue myregdata = mydearpayloaddata.value("data");
            QJsonArray myqueryarr = myregdata.toArray();
            QJsonDocument responsedoc(myqueryarr);
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = responsedoc.toJson(QJsonDocument::Compact);
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " fetch all details by user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " fetch all details by user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


        }



    });


    // End fetch all details


    // Start admin operations

    http.dispatch("POST", "/deliva/universal/adminoperations", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        //  QString queryparam =  request.params().value("reason");
        //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        //  QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/erradminoperations.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccadminoperations.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString strPayload = QString::fromUtf8(request.data());
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myentity =  mydearpayload.value("entity").toString();
        QString myaction =  mydearpayload.value("action").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myreason =  request.params().value("reason");
        QString myusername =  ((myrequest.value("username").toString()=="" || myrequest.value("username").toString().isNull())  ?  myrequest.value("username").toString() : myreason) ;
        // qDebug() << "myreason is: " << myreason << Qt::endl;
        QMap<QString, QVariant> params0;
        params0["username"] = myusername;   // add field

        QString where0 =  buildWhereClause(params0);
        // -> "username = 'Alice'"

      //  QString whereids =buildWhereClause(params0,QSet<QString>(),"ids");

        QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        qx::QxRestApi cryptoapi;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where0);
        QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
        qx::QxRestApi cryptoapiadsupwebjson;
        QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
        // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
        mypayload->processRequest(resultcryptoadsupwebjson);
        QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
        QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
        QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
        QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
        QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
        // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
        //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
        qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
        //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





        if((adsupwebjsonroles>=2)) {



            qx::QxRestApi dataapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());
          //  modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query.sql", whereids); // added
           // modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("ids")); // added
            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {

            // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }



            QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
            // QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
            //  qDebug() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
            mypayload->processRequest(resultdata);
            QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
            QJsonValue myregdata = mydearpayloaddata.value("data");
            QJsonArray myqueryarr = myregdata.toArray();
            QJsonDocument responsedoc(myqueryarr);
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = responsedoc.toJson(QJsonDocument::Compact);
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " Admin operation by admin " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " Admin operation by admin " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since the user is not admin." ;


        }



    });


    // End admin operations


    // Start core super power operations

    http.dispatch("POST", "/deliva/universal/coreoperations", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        //  QString queryparam =  request.params().value("reason");
        //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
        //  QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
        //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errcoreoperations.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/scccoreoperations.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString strPayload = QString::fromUtf8(request.data());
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        // QString myentity =  mydearpayload.value("entity").toString();
        // QString myaction =  mydearpayload.value("action").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myreason =  request.params().value("reason");
        QString myusername =  ((myrequest.value("username").toString()=="" || myrequest.value("username").toString().isNull())  ?  myrequest.value("username").toString() : myreason) ;
        QMap<QString, QVariant> params0;
        params0["username"] = myusername;   // add field

        QString where0 = buildWhereClause(params0);
        // -> "username = 'Alice'"

      //  QString whereids =buildWhereClause(params0,QSet<QString>(),"ids");


        QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        qx::QxRestApi cryptoapi;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where0);
        QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        // qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
        qx::QxRestApi cryptoapiadsupwebjson;
        QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
        // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
        mypayload->processRequest(resultcryptoadsupwebjson);
        QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
        QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
        QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
        QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
        QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
        // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
        //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
        qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
        //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;


        if((adsupwebjsonroles>=4)) {



            qx::QxRestApi dataapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());
           // modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query.sql", whereids); // added but might not be useful
           // modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("ids")); // added
            QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
            //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
            //  qDebug() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
            mypayload->processRequest(resultdata);
            QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
            QJsonValue myregdata = mydearpayloaddata.value("data");
            QJsonArray myqueryarr = myregdata.toArray();
            QJsonDocument responsedoc(myqueryarr);
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = responsedoc.toJson(QJsonDocument::Compact);
            errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            sccentry << " super power operations  by superpower " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " ." ;
            errentry << " super power operations  by superpower " << myrequest.value("username").toString().toStdString() <<" at "
                     << myrequest.value("created_at").toString().toStdString() << " was not successful since the user is not admin." ;


        }



    });


    // End core super power operations

    // Start get all ids for a user

    http.dispatch("GET", "/deliva/fetchids/<user>", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::string starter1 = R"(

        {
            "action": "fetch_all",
            "data":
            {
                "created_at": "2022-07-28T15:19:09.733Z",
                "token": "504601637",
                "username": "ericm"
            },
            "entity": "categories",
            "query":
            {
                "sql": "username = 'ericm'"
            }
        }

        )";


        std::string starter2 = R"(

        {
            "action": "fetch_by_query",
            "data":
            {
                "created_at": "2022-07-28T15:19:09.733Z",
                "token": "504601637",
                "username": "auser"
            },
            "entity": "ids",
            "query":
            {
                "sql": "username = 'auser'"
            },
            "relations": [
                "*"
            ]
        }

        )";


        //  QString queryparam =  request.params().value("reason");
        QString urlparam =  request.dispatchParams().value("user").toString();
        //  qDebug() << "query param for fetchids is: " << queryparam << Qt::endl;
        //  qDebug() << "url param for fetchids is: " << urlparam << Qt::endl;
        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchids.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchids.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString strPayload = QString::fromStdString(starter2);
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QString myentity =  mydearpayload.value("entity").toString();
        QString myaction =  mydearpayload.value("action").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myreason =  request.params().value("reason");
        //  qDebug() << "myreason is: " << myreason << Qt::endl;
        QMap<QString, QVariant> params0;
        params0["username"] = myreason;

        QString where0 = buildWhereClause(params0);
        // -> "username = 'value_of_myreason'"

        QMap<QString, QVariant> params1;
        params1["username"] = urlparam;

        QString where1 = buildWhereClause(params1,QSet<QString>(),"ids");
        // -> "username = 'value_of_urlparam'"


        QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        qx::QxRestApi cryptoapi;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where0);
        QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
        qx::QxRestApi cryptoapiadsupwebjson;
        QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
        // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
        mypayload->processRequest(resultcryptoadsupwebjson);
        QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
        QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
        QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
        QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
        QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
        // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
        //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
        qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
        //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





        if((adsupwebjsonroles>=2)) {



            qx::QxRestApi dataapi;
            QJsonDocument usagedoc = QJsonDocument::fromJson(strPayload.toUtf8());
            modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query.sql", where1); // added
          //  modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("ids")); // added

            //  QString adsupwebfinaljson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
            // qDebug() << "adsupwebfinaljson after modification is: " << qPrintable(adsupwebfinaljson) << Qt::endl;


            // if((myentity=="ids" || myentity=="cryptocenter"  ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
            if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }

            if((myentity=="mysecrets" ) ) {

                modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

            }


            QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
            QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
          //  qInfo() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
            mypayload->processRequest(resultdata);
            QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
            QJsonValue myregdata = mydearpayloaddata.value("data");
            QJsonArray myqueryarr = myregdata.toArray();
            QJsonDocument responsedoc(myqueryarr);
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = responsedoc.toJson(QJsonDocument::Compact);
            errentry << " No error detected for user "  << urlparam.toStdString() <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            sccentry << " fetch ids for user " << urlparam.toStdString()  <<" at "
                     <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;




        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for user " << urlparam.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            errentry << " fetch ids for user " << urlparam.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not admin." ;


        }



    });


    // End get all ids for a user


    // Start get all data from ids entity as superpower

    http.dispatch("GET", "/deliva/fetchidssuperpower/<param>", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {


        std::string starter1 = R"({
                             "action": "fetch_by_query",
                             "data":
                             {
                                 "created_at": "2022-07-28T15:19:09.733Z",
                                 "token": "504601637",
                                 "username": "auser"
                             },
                            "entity": "ids",
                             "query":
                             {
                                 "sql": "{\"username\":\"auser\"}"
                             },
                           "relations": [
                                   "*"
                                ]

                         })";





        QStringList m_destlist;
        m_destlist << "*";
        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        QJsonArray m_array = QJsonArray::fromStringList(m_destlist);
        QString urlparam =  request.dispatchParams().value("param").toString();
        Wt::WLogger errlogger;
        errlogger.addField("superpowererrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchidssuperpower.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchidssuperpower.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString strPayload = QString::fromStdString(starter1);
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myreason =  request.params().value("reason");
        //  qDebug() << "myreason is: " << myreason << Qt::endl;
        QMap<QString, QVariant> params0;
        params0["username"] = myreason;   // add the field-value

        QString where0 = buildWhereClause(params0);
        // Result example: "username = 'john'"

       // QString whereids =buildWhereClause(params0,QSet<QString>(),"ids");


        QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        qx::QxRestApi cryptoapi;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where0);
        QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
        qx::QxRestApi cryptoapiadsupwebjson;
        QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
        // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
        mypayload->processRequest(resultcryptoadsupwebjson);
        QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
        QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
        QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
        QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
        QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
        // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
        //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
        qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
        //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





        if((adsupwebjsonroles>=4) && (urlparam=="all")) {



            qx::QxRestApi dataapi;
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_all"));
           // modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", whereids); // added but might not be useful
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations", m_array);
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("ids"));
            QString adsupwebfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
            qInfo() << "adsupwebfinaljson superpower ids after modification is: " << qPrintable(adsupwebfinaljson) << Qt::endl;




            QString resultdata = dataapi.processRequest(userdoc.toJson(QJsonDocument::Indented));
            //   QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
            //  qDebug() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
            mypayload->processRequest(resultdata);
            QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
            QJsonValue myregdata = mydearpayloaddata.value("data");
            QJsonArray myqueryarr = myregdata.toArray();
            QJsonDocument responsedoc(myqueryarr);
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = responsedoc.toJson(QJsonDocument::Compact);
            errentry << " No error detected for fetch all users from entity ids by superpower  "  << myreason.toStdString() <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            sccentry << " fetch all users from entity ids by superpower  " << myreason.toStdString()  <<" at "
                     <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;



        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for fetch all users from entity ids by superpower " << myreason.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            errentry << " fetch all users from entity ids by superpower "  << myreason.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not a superpower." ;


        }



    });


    // End get all data from ids entity as superpower


    // Start get all data from cryptocenter entity as superpower

    http.dispatch("GET", "/deliva/fetchcryptosuperpower/<param>", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {



        std::string starter1 = R"({
                             "action": "fetch_by_query",
                             "data":
                             {
                                 "created_at": "2022-07-28T15:19:09.733Z",
                                 "token": "504601637",
                                 "username": "auser"
                             },
                            "entity": "ids",
                             "query":
                             {
                                 "sql": "{\"username\":\"auser\"}"
                             },
                           "relations": [
                                   "*"
                                ]

                         })";




        QString urlparam =  request.dispatchParams().value("param").toString();
        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("superpowererrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchcryptosuperpower.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchcryptosuperpower.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QString strPayload = QString::fromStdString(starter1);
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myreason =  request.params().value("reason");
        //  qDebug() << "myreason is: " << myreason << Qt::endl;
        QMap<QString, QVariant> params0;
        params0["username"] = myreason;   // will be skipped if myreason is empty/null


        QString where0 = buildWhereClause(params0);
      //  QString whereids =buildWhereClause(params0,QSet<QString>(),"ids");

        QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        qx::QxRestApi cryptoapi;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where0);
        QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
        qx::QxRestApi cryptoapiadsupwebjson;
        QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
        // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
        mypayload->processRequest(resultcryptoadsupwebjson);
        QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
        QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
        QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
        QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
        QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
        // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
        //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
        qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
        //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





        if((adsupwebjsonroles>=4) && (urlparam=="all")) {



            qx::QxRestApi dataapi;
          //  modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", whereids); // added but might not be useful
           // modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("ids")); // added
            modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_all"));
            QString adsupwebfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
            qDebug() << "adsupwebfinaljson superpower cryptocenter after modification is: " << qPrintable(adsupwebfinaljson) << Qt::endl;
            QString resultdata = dataapi.processRequest(userdoc.toJson(QJsonDocument::Indented));
            //   QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
            //  qDebug() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
            mypayload->processRequest(resultdata);
            QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
            QJsonValue myregdata = mydearpayloaddata.value("data");
            QJsonArray myqueryarr = myregdata.toArray();
            QJsonDocument responsedoc(myqueryarr);
            response.headers().insert("Content-Type", "application/json; charset=utf-8");
            response.data() = responsedoc.toJson(QJsonDocument::Compact);
            errentry << " No error detected for fetch all users from entity cryptocenter by superpower  "  << myreason.toStdString() <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            sccentry << " fetch all users from entity cryptocenter by superpower  " << myreason.toStdString()  <<" at "
                     <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;




        }

        else {


            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for fetch all users from entity cryptocenter by superpower " << myreason.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            errentry << " fetch all users from entity cryptocenter by superpower "  << myreason.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not a superpower." ;


        }



    });


    // End get all data from cryptocenter entity as superpower


    // Start get token register or login


    http.dispatch("POST", "/deliva/universal/gettoken", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errgettoken.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccgettoken.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QByteArray utfString = request.data();
        QString strPayload = QString::fromUtf8(utfString);
        QString queryparam =  request.params().value("reason");
        //  QString mytoken = queryparam;
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        //  QString myfield =  mydearpayload.value("field").toString();
        // QString myentity =  mydearpayload.value("entity").toString();
        QString mysecret =  mydearpayload.value("secret").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myreason =  request.params().value("reason");
        QString myusername =  myreason ;
        //  qDebug() << "myreason is: " << myreason << Qt::endl;
        qDebug() << "mysecret is: " << mysecret << Qt::endl;
        QMap<QString, QVariant> params0;
        params0["username"] = myusername;

        QString where0 = buildWhereClause(params0);
        // Example output:  username = 'john'

        QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        qx::QxRestApi cryptoapi;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where0);
        QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
        qx::QxRestApi cryptoapiadsupwebjson;
        QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
        // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
        mypayload->processRequest(resultcryptoadsupwebjson);
        QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
        QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
        QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
        QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
        QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
        // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
        //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
        qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
        //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;

        if((adsupwebjsonroles>=2)) {


            eric::JsonUtils m;
            QVariantMap usagemap = m.toVariantMap(strPayload);
            QString desttoken = modifyMyJsonToolsuser->jwttoken(usagemap,mysecret);
            //   qDebug() << "desttoken is: " << desttoken << Qt::endl;
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 200;
            response.data() = desttoken.toUtf8();

            errentry << " No error detected for admin "  << queryparam.toStdString() <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            sccentry << " get token by admin " << queryparam.toStdString()  <<" at "
                     <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;



        }

        else {

            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for admin " << queryparam.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            errentry << " get token by admin " << queryparam.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not admin." ;



        }




    });


    // End get token register or login


    // Start get reason from token register


    http.dispatch("POST", "/deliva/universal/getreasontoken", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

        std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
        Wt::WLogger errlogger;
        errlogger.addField("systemerrors", false);
        errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errgetreasontoken.log");
        Wt::WLogEntry errentry = errlogger.entry("error");
        Wt::WLogger scclogger;
        scclogger.addField("accessgranted", true);
        scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccgetreasontoken.log");
        Wt::WLogEntry sccentry = scclogger.entry("info");
        QByteArray utfString = request.data();
        //  QJsonDocument mydoc = QJsonDocument::fromJson(utfString);
        //  qDebug() << "Json Doc in getreasontoken is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
        QString strPayload = QString::fromUtf8(utfString);
        QString queryparam =  request.params().value("reason");
        //  QString mytoken = queryparam;
        mypayload->processRequest(strPayload);
        QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
        //  QString myfield =  mydearpayload.value("field").toString();
        // QString myentity =  mydearpayload.value("entity").toString();
        QString mysecret =  mydearpayload.value("secret").toString();
        QJsonValue myvalue = mydearpayload.value("data");
        QJsonObject myrequest = myvalue.toObject();
        QString myreason =  request.params().value("reason");
        QString tokenuser = myrequest.value("username").toString();
        QString tokenjwt = myrequest.value("jwt").toString();
        QString myusername =  myreason;
        //  qDebug() << "myreason is: " << myreason << Qt::endl;
        QMap<QString, QVariant> params0;
        params0["username"] = myusername;

        QString where0 = buildWhereClause(params0);
        // Example output:  username = 'bob'

        QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
        std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
        qx::QxRestApi cryptoapi;
        modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where0);
        QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
        //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
        qx::QxRestApi cryptoapiadsupwebjson;
        QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
        // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
        mypayload->processRequest(resultcryptoadsupwebjson);
        QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
        QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
        QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
        QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
        QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
        // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
        //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
        qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
        //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;

        if((adsupwebjsonroles>=2)) {



            QString destreason = modifyMyJsonToolsuser->extracttoken(tokenjwt,tokenuser,mysecret);
            //   qDebug() << "destreason is: " << destreason << Qt::endl;
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 200;
            response.data() = destreason.toUtf8();

            errentry << " No error detected for admin "  << queryparam.toStdString() <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            sccentry << " get reason from token by admin " << queryparam.toStdString()  <<" at "
                     <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;



        }

        else {

            const std::string ErrorTemplate = "denied";
            QString result = (QString::fromStdString(ErrorTemplate));
            response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
            response.status() = 403;
            response.data() = result.toUtf8();
            sccentry << " Error detected for admin " << queryparam.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
            errentry << " get reason from token by admin " << queryparam.toStdString()  <<" at "
                     << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not admin." ;



        }




    });
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
    url_ = QStringLiteral("http://%1:%2/").arg(listenIp_,port_);
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
            cfg->setIp(listenIp_);
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

   // qInfo() << "where0 is: " << where0 << Qt::endl;

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








// // QxHttpServerController.cpp
// #include "QxHttpServerController.h"

// #include <QDebug>
// #include <QDir>
// #include <QStandardPaths>
// #include <QFile>
// #include <QTextStream>
// #include <QNetworkInterface>
// #include <QTcpServer>
// #include <QTcpSocket>

// #include <QSqlDatabase>
// #include <QSqlQuery>
// #include <QSqlError>

// #include <QSslCertificate>
// #include <QSslKey>

// #include <QThread>

// // QxOrm / QxService headers (adjust include paths to your project)
// #include <QxRegister/QxClassX.h>
// #include <QxService/QxConnect.h>
// #include <QxService/QxThreadPool.h>

// // If you want to attempt generator-based creation, define USE_QX_GENERATOR in your CMake,
// // and ensure the generator header path is on the include path.
// #ifdef USE_QX_GENERATOR
// // Adjust this include to your actual header path if different
// #include "QxDao/QxSqlGenerator/QxSqlGenerator_SQLite.h"
// #endif

// // Your model includes (keep as you had them)
// #include "orm/orders.h"
// #include "orm/locations.h"
// #include "orm/logistics.h"
// #include "orm/delivery.h"
// #include "orm/ids.hpp"
// #include "orm/cryptocenter.hpp"
// #include "orm/mysecrets.h"
// #include "orm/metrics.hpp"
// #include "orm/categories.hpp"



// #include "precompiled.h"
// #include "MyJsonTools.hpp"



// QVariantMap mysecretstable(qx::QxHttpRequest & request);


// // ------------------- helper: execute SQL script -------------------
// bool QxHttpServerController::executeSqlScript(QSqlDatabase &db, const QString &scriptContents)
// {
//     QStringList statements = scriptContents.split(';', Qt::SkipEmptyParts);
//     QSqlQuery q(db);
//     for (QString stmt : statements) {
//         stmt = stmt.trimmed();
//         if (stmt.isEmpty()) continue;
//         if (!q.exec(stmt)) {
//             qWarning() << "[SQL] Statement failed:" << q.lastError().text() << "\nSQL (truncated):" << stmt.left(2000);
//             return false;
//         } else {
//             qInfo() << "[SQL] Executed statement (len)" << stmt.length();
//         }
//     }
//     return true;
// }

// // ------------------- ctor/dtor -------------------
// QxHttpServerController::QxHttpServerController(QObject *parent)
//     : QObject(parent)
// {
// }

// QxHttpServerController::~QxHttpServerController()
// {
//     stopServer();
// }

// // ------------------- util: ips -------------------
// QStringList QxHttpServerController::availableIpAddresses() const
// {
//     QStringList ips;
//     for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
//         if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
//         if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
//         for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
//             QHostAddress addr = entry.ip();
//             if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
//                 ips << addr.toString();
//             }
//         }
//     }
//     ips.removeDuplicates();
//     if (!ips.contains(QStringLiteral("127.0.0.1")))
//         ips.prepend(QStringLiteral("127.0.0.1"));
//     return ips;
// }

// void QxHttpServerController::setPort(int p)
// {
//     if (p == port_) return;
//     port_ = p;
//     Q_EMIT portChanged();
// }

// // ------------------- sqlite configuration -------------------
// bool QxHttpServerController::configureSqlite()
// {
// #if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
//     QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
// #else
//     QString base = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
// #endif
//     if (base.isEmpty()) base = QDir::currentPath();

//     QDir().mkpath(base);
//     sqlitePath_ = QDir(base).filePath("deliva.db");
//     qInfo() << "[QxHttpServerController] SQLite DB file:" << sqlitePath_;

//     const QString connName = QStringLiteral("qx_http_main");
//     if (QSqlDatabase::contains(connName)) QSqlDatabase::removeDatabase(connName);

//     QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
//     db.setDatabaseName(sqlitePath_);
//     if (!db.open()) {
//         qWarning() << "[QxHttpServerController] Failed to open SQLite DB:" << db.lastError().text();
//         QSqlDatabase::removeDatabase(connName);
//         return false;
//     }

//     QSqlQuery q(db);
//     if (!q.exec("PRAGMA journal_mode = WAL;")) {
//         qWarning() << "[QxHttpServerController] PRAGMA journal_mode WAL failed:" << q.lastError().text();
//     } else {
//         qInfo() << "[QxHttpServerController] PRAGMA journal_mode = WAL";
//     }
//     if (!q.exec("PRAGMA synchronous = NORMAL;"))
//         qWarning() << "[QxHttpServerController] PRAGMA synchronous NORMAL failed:" << q.lastError().text();
//     if (!q.exec("PRAGMA temp_store = MEMORY;"))
//         qWarning() << "[QxHttpServerController] PRAGMA temp_store MEMORY failed:" << q.lastError().text();

//     // Try resource SQL first (robust)
//     const QString resourcePath = QStringLiteral(":/sql/sqlite_schema.sql");
//     if (QFile::exists(resourcePath)) {
//         qInfo() << "[QxHttpServerController] Found packaged sqlite_schema.sql; applying schema...";
//         QFile f(resourcePath);
//         if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
//             qWarning() << "[QxHttpServerController] Failed opening resource" << resourcePath << ":" << f.errorString();
//         } else {
//             QTextStream in(&f);
//             const QString scriptContents = in.readAll();
//             f.close();
//             if (!executeSqlScript(db, scriptContents)) {
//                 qWarning() << "[QxHttpServerController] Failed to execute schema script from resource";
//             } else {
//                 qInfo() << "[QxHttpServerController] Schema script applied successfully";
//             }
//         }
//     } else {
//         qInfo() << "[QxHttpServerController] No packaged sqlite_schema.sql found in resources; will rely on qx::dao::create_table calls later";
//     }

//     db.close();
//     QSqlDatabase::removeDatabase(connName);
//     return true;
// }

// // ------------------- ssl config -------------------
// bool QxHttpServerController::configureSsl(const QString &appPath)
// {
//     qInfo() << "[SSL] configureSsl: appPath =" << appPath;
//     const QString resCA  = QStringLiteral(":/encrypt/DonaterootCA.crt");
//     const QString resCrt = QStringLiteral(":/encrypt/donate.crt");
//     const QString resKey = QStringLiteral(":/encrypt/donate.key");
//     const QString resDh  = QStringLiteral(":/encrypt/dhparam.pem"); // optional

//     QDir dir(appPath);
//     if (!dir.exists("files")) dir.mkpath("files");

//     const QString outCA  = dir.filePath("files/DonaterootCA.crt");
//     const QString outCrt = dir.filePath("files/donate.crt");
//     const QString outKey = dir.filePath("files/donate.key");
//     const QString outDh  = dir.filePath("files/dhparam.pem");

//     auto copyRes = [](const QString &src, const QString &dst) -> bool {
//         if (!QFile::exists(src)) {
//             qWarning() << "[SSL] Resource missing:" << src;
//             return false;
//         }
//         QFile dest(dst);
//         if (dest.exists()) dest.remove();
//         if (!QFile::copy(src, dst)) {
//             qWarning() << "[SSL] Failed to copy" << src << "->" << dst;
//             return false;
//         }
//         return true;
//     };

//     bool okCA = copyRes(resCA, outCA);
//     bool okCrt = copyRes(resCrt, outCrt);
//     bool okKey = copyRes(resKey, outKey);
//     bool okDh = true;
//     if (QFile::exists(resDh)) {
//         okDh = copyRes(resDh, outDh);
//     } else {
//         qInfo() << "[SSL] dhparam.pem not present in resources; continuing without it";
//     }

//     if (!okCA || !okCrt || !okKey) {
//         qWarning() << "[SSL] Required cert/key resources missing or failed to copy - SSL disabled";
//         return false;
//     }

//     QFile fCA(outCA);
//     if (!fCA.open(QIODevice::ReadOnly)) {
//         qWarning() << "[SSL] Cannot open CA cert file:" << outCA << fCA.errorString();
//         return false;
//     }
//     QList<QSslCertificate> caList;
//     caList << QSslCertificate(fCA.readAll());
//     fCA.close();

//     QFile fCrt(outCrt);
//     if (!fCrt.open(QIODevice::ReadOnly)) {
//         qWarning() << "[SSL] Cannot open server cert file:" << outCrt << fCrt.errorString();
//         return false;
//     }
//     QSslCertificate serverCert(fCrt.readAll());
//     fCrt.close();

//     QFile fKey(outKey);
//     if (!fKey.open(QIODevice::ReadOnly)) {
//         qWarning() << "[SSL] Cannot open server key file:" << outKey << fKey.errorString();
//         return false;
//     }
//     const QByteArray keyData = fKey.readAll();
//     fKey.close();

//     const QByteArray keyPassphrase = QByteArrayLiteral("ericpass"); // dev only
//     QSslKey serverKey(keyData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, keyPassphrase);
//     if (serverKey.isNull()) {
//         qWarning() << "[SSL] Failed to parse private key. Is the passphrase correct or key format supported?";
//         return false;
//     }

//     qx::service::QxConnect *serverSettings = qx::service::QxConnect::getSingleton();
//     if (!serverSettings) {
//         qWarning() << "[SSL] QxConnect singleton not available";
//         return false;
//     }

//     serverSettings->setSSLEnabled(true);
//     serverSettings->setSSLCACertificates(caList);
//     serverSettings->setSSLLocalCertificate(serverCert);
//     serverSettings->setSSLPrivateKey(serverKey);

//     if (okDh) qInfo() << "[SSL] dhparam.pem copied to" << outDh;

//     qInfo() << "[SSL] SSL configured (self-signed/dev CA).";
//     return true;
// }

// // ------------------- routes (minimal) -------------------
// void QxHttpServerController::setupRoutes(qx::QxHttpServer &http)
// {
//     // Paste your dispatch(...) blocks from main.cpp here.
//     // Minimal example:
//     // Start create new account

//     http.dispatch("POST", "/deliva/ids/new", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errlog.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/scclog.log");
//         Wt::Json::Object bodyContent;
//         Wt::Json::Object tokenContent;
//         Wt::Json::parse(QString::fromUtf8(request.data()).toStdString(), tokenContent);
//         std::string registerjwt = tokenContent.get("token");
//         // std::cout << "register jwt is: " << registerjwt << std::endl;
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         //  std::unique_ptr<MyJsonTools> toolidsnew {new MyJsonTools};
//         //  toolidsnew->processRequest(request.data());
//         //  QJsonObject myrequest = toolidsnew->m_requestJson.toObject();
//         // QString mytokendata = myrequest.value("token").toString();
//         // qDebug() << "my token data is: " << mytokendata << Qt::endl;
//         QString strToken = QString::fromStdString(registerjwt);
//         QStringList listJwtParts = strToken.split(".");
//         bool tokenformatcorrect = true;
//         if (listJwtParts.count() != 3){
//             // token format error
//             tokenformatcorrect = false;
//             errentry << "bad token";


//         }

//         QVariantMap extractsecrets = mysecretstable(request);
//         QString strSecret =  extractsecrets.value("secretregister").toString();
//         if (strSecret.isEmpty())
//         {

//         }

//         QJsonWebToken token = QJsonWebToken::fromTokenAndSecret(strToken, strSecret);
//         bool isvalidtoken = false;
//         // get decoded header and payload
//         // QString strHeader = token.getHeaderQStr();
//         QString strPayload = "";
//         // QString jsonobj ="";

//         if (token.isValid()) {

//             strPayload = token.getPayloadQStr();
//             // qDebug() <<"valid token" << Qt::endl;
//             isvalidtoken=true;


//         }


//         else {


//             errentry << "invalid token";
//             //  qDebug() <<"invalid token" << Qt::endl;




//         }

//         // qDebug() << "Payload is: " << strPayload << Qt::endl;

//         //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QString myaction =  mydearpayload.value("action").toString();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QJsonObject myrequest = myvalue.toObject();
//         QString myusername =  myrequest.value("username").toString();
//         QString mypassword =  myrequest.value("password").toString();
//         qint64 mypolicy =  myrequest.value("policy").toInt();

//         if(isvalidtoken && tokenformatcorrect ) {


//             //  qDebug() << "my policy is: " << mypolicy << Qt::endl;
//             /*

//      QString myroles =  myrequest.value("roles").toString();
//      QString mycreated_at =  myrequest.value("created_at").toString();
//      QByteArray myadminnames =  myrequest.value("adminnames").toString().toUtf8();
//      QString myuuid =  myrequest.value("uuid").toString();
//      qDebug() << "my role is: " << myroles << Qt::endl;
//      qDebug() << "my uuid is: " << myuuid << Qt::endl;
//      qDebug() << "my created_at is: " << mycreated_at << Qt::endl;
//      qDebug() << "my adminnames are: " << myadminnames << Qt::endl;
//      QUuid m = QUuid(myuuid);
//      if(m.isNull()){ qDebug() << "null uuid " << Qt::endl;}
//      qDebug() << "my converted uuid is: " << m << Qt::endl;
//      qDebug() << "my converted uuid to string is: " << m.toString(QUuid::WithoutBraces) << Qt::endl;
//      QString myidids = myrequest.value("ids_id").toString();
//      qDebug() << "myidids is: " << myidids << Qt::endl;

//    */


//             QString user1 = myusername;
//             QString slash1 = + "\"" + user1 + "\"";
//             QString query1 = "{ \"username\" : " + slash1 + " }";
//             QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
//             // qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//             std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", "fetch_by_query");
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", "cryptocenter");
//             qx::QxRestApi queryapi;
//             QString result = queryapi.processRequest(userdoc.toJson(QJsonDocument::JsonFormat::Compact));
//             // qDebug() << "validate query json is: " << result << Qt::endl;
//             //  const QJsonDocument mydoc = QJsonDocument::fromJson(result.toUtf8());
//             // qDebug() << "results as json doc are: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Compact)) << Qt::endl;

//             std::shared_ptr<MyJsonTools> mypayloadquery {new MyJsonTools};
//             mypayloadquery->processRequest(result);
//             QJsonObject mydearquery = mypayloadquery->m_requestJson.toObject();
//             QJsonValue myvaluefromquery = mydearquery.value("data");
//             QJsonArray myrequestfromqueryarr = myvaluefromquery.toArray();
//             QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//             QJsonObject myrequestfromquery = dataFirstarr.toObject();
//             QString queryresults =  myrequestfromquery.value("username").toString();
//             //  qDebug() << "query response is: " << queryresults << Qt::endl;

//             if(!(queryresults==myusername)) {

//                 qx::QxRestApi loadapi;
//                 QJsonDocument modifymydoc = QJsonDocument::fromJson(strPayload.toUtf8());
//                 std::shared_ptr<MyJsonTools>  modifymyMyJsonTools {new MyJsonTools};
//                 modifymyMyJsonTools->resetadmin(modifymydoc, modifymyMyJsonTools,2);
//                 // if((myentity=="ids" || myentity=="cryptocenter"  ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//                 if((myentity=="mysecrets" ) ) {

//                     modifymyMyJsonTools->modifyJsonValue(modifymydoc, "entity", QJsonValue("nullandvoid"));

//                 }

//                 QString myidsjson = modifymydoc.toJson(QJsonDocument::JsonFormat::Indented);
//                 //  qDebug() << "myidsjson after reset is: " << qPrintable(myidsjson) << Qt::endl;
//                 QString resultreg = loadapi.processRequest(myidsjson);
//                 //  qDebug() << "result after registration in ids is: " << resultreg << Qt::endl;


//                 QVariantMap mysecurity = modifymyMyJsonTools->encryptpass(mypassword.toStdString());
//                 QString mycryptokey = mysecurity.value("key").toString();
//                 modifymyMyJsonTools->modifyJsonValue(modifymydoc, "entity", QJsonValue("cryptocenter"));
//                 modifymyMyJsonTools->modifyJsonValue(modifymydoc, "query.entity", QJsonValue("cryptocenter"));

//                 if((myentity=="mysecrets" ) ) {

//                     modifymyMyJsonTools->modifyJsonValue(modifymydoc, "entity", QJsonValue("nullandvoid"));
//                     modifymyMyJsonTools->modifyJsonValue(modifymydoc, "query.entity", QJsonValue("nullandvoid"));

//                 }


//                 if(mypolicy==0) {


//                     modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.key", QJsonValue(mycryptokey));

//                 }

//                 else {


//                     modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.key", QJsonValue("policyretain"));


//                 }

//                 modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.iv",  QJsonValue(mysecurity.value("iv").toString()));
//                 modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.cipher",  QJsonValue(mysecurity.value("cipher").toString()));
//                 QRandomGenerator64 *gen = QRandomGenerator64::system();
//                 gen->securelySeeded();
//                 long long discardx = gen->bounded(1000000000);
//                 // qDebug() << "my token  is: " << discardx << Qt::endl;
//                 modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.token", QJsonValue(discardx));
//                 QString myfinaljson = modifymydoc.toJson(QJsonDocument::JsonFormat::Compact);
//                 //  QString myfinaljson = modifymydoc.toJson(QJsonDocument::JsonFormat::Indented);
//                 // qDebug() << "my final json after modification is: " << qPrintable(myfinaljson) << Qt::endl;

//                 qx::QxRestApi cryptoapi;
//                 QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//                 Q_UNUSED(resultcrypto)
//                 //  qDebug() << "result after registration in crypto is: " << resultcrypto << Qt::endl;
//                 mypayload->processRequest(resultreg);
//                 QJsonObject myregpayload = mypayload->m_requestJson.toObject();
//                 QJsonValue myregvalue = myregpayload.value("data");
//                 QJsonObject myregrequest = myregvalue.toObject();
//                 QString myids_id =  myregrequest.value("ids_id").toString();
//                 // qDebug() << "my ids_id is: " << myids_id << Qt::endl;
//                 QJsonWebToken m_jwtObj;
//                 QDateTime mycurrent = QDateTime::currentDateTime();
//                 QString mydate = mycurrent.toString(Qt::ISODate);
//                 m_jwtObj.removeAll();
//                 m_jwtObj.appendClaim(myusername,  QString::number(discardx));
//                 m_jwtObj.setAlgorithmStr("HSSha3_512");
//                 m_jwtObj.setSecret(strSecret);
//                 QJsonArray newarr;
//                 QJsonObject object;
//                 object.insert("token", m_jwtObj.getToken());
//                 object.insert("ids_id", myids_id);
//                 if((mypolicy==1)) {

//                     object.insert("key", mycryptokey);

//                 }
//                 object.insert("date", QString::fromStdString(mydate.toStdString()));
//                 newarr << object;
//                 QJsonDocument responsedoc(newarr);
//                 //  QString myresponsedocjson = responsedoc.toJson(QJsonDocument::JsonFormat::Indented);
//                 // qDebug() << "myresponsedocjson from register is: " << qPrintable(myresponsedocjson) << Qt::endl;
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);
//                 errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                          << myrequest.value("created_at").toString().toStdString() << " ." ;
//                 sccentry << " Registration of new user " << myrequest.value("username").toString().toStdString() <<" at "
//                          << myrequest.value("created_at").toString().toStdString() << " was successful." ;





//             }

//             else {


//                 const std::string ErrorTemplate = "alreadyexists";
//                 QString result = (QString::fromStdString(ErrorTemplate));
//                 response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//                 response.status() = 403;
//                 response.data() = result.toUtf8();
//                 sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                          << myrequest.value("created_at").toString().toStdString() << " ." ;
//                 errentry << " Registration of new user " << myrequest.value("username").toString().toStdString() <<" at "
//                          << myrequest.value("created_at").toString().toStdString() << " was not successful since the user existed." ;


//             }






//         }

//         else {

//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " Registration of new user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;




//         }






//     });

//     // End create new account



//     // Start login user

//     http.dispatch("POST", "/deliva/ids/login", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {


//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errlog.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/scclog.log");
//         Wt::Json::Object bodyContent;
//         Wt::Json::Object tokenContent;
//         Wt::Json::parse(QString::fromUtf8(request.data()).toStdString(), tokenContent);
//         std::string loginjwt = tokenContent.get("token");
//         //  std::cout << "login jwt is: " << loginjwt << std::endl;
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         //  std::unique_ptr<MyJsonTools> toolidsnew {new MyJsonTools};
//         //  toolidsnew->processRequest(request.data());
//         //  QJsonObject myrequest = toolidsnew->m_requestJson.toObject();
//         // QString mytokendata = myrequest.value("token").toString();
//         // qDebug() << "my token data is: " << mytokendata << Qt::endl;
//         QString strToken = QString::fromStdString(loginjwt);
//         QStringList listJwtParts = strToken.split(".");
//         bool tokenformatcorrect = true;
//         if (listJwtParts.count() != 3){
//             // token format error
//             tokenformatcorrect = false;
//             errentry << "bad token";


//         }

//         QVariantMap extractsecrets = mysecretstable(request);
//         QString strSecret =  extractsecrets.value("secretlogin").toString();
//         if (strSecret.isEmpty())
//         {

//         }

//         QJsonWebToken token = QJsonWebToken::fromTokenAndSecret(strToken, strSecret);
//         bool isvalidtoken = false;
//         // get decoded header and payload
//         // QString strHeader = token.getHeaderQStr();
//         QString strPayload = "";
//         // QString jsonobj ="";

//         if (token.isValid()) {

//             strPayload = token.getPayloadQStr();
//             // qDebug() <<"valid token" << Qt::endl;
//             isvalidtoken=true;


//         }


//         else {


//             errentry << "invalid token";
//             //  qDebug() <<"invalid token" << Qt::endl;




//         }


//         //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myusername =  myrequest.value("username").toString();
//         QString mypassword =  myrequest.value("password").toString();

//         if(isvalidtoken && tokenformatcorrect ) {



//             QString user1 = myusername;
//             QString slash1 = + "\"" + user1 + "\"";
//             QString query1 = "{ \"username\" : " + slash1 + " }";
//             QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
//             // qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//             std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//             // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
//             QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
//             qx::QxRestApi cryptoapi;
//             QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//             Q_UNUSED(resultcrypto)
//             //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
//             mypayload->processRequest(resultcrypto);
//             QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//             QJsonValue myregvalue = mydearpayloadresults.value("data");
//             QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//             QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//             QJsonObject myrequestfromquery = dataFirstarr.toObject();
//             // QString queryusername =  myrequestfromquery.value("username").toString();
//             QString queryiv =  myrequestfromquery.value("iv").toString();
//             QString querycipher =  myrequestfromquery.value("cipher").toString();
//             QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//             QString mykey = "none";
//             qint64 querypolicy =  myrequestfromquery.value("policy").toInt();

//             if(querypolicy==0) {

//                 mykey = myrequestfromquery.value("key").toString();

//             }

//             else {

//                 mykey = myrequest.value("key").toString();

//             }

//             QVariantMap v;
//             v.insert("key", mykey);
//             v.insert("iv", queryiv);
//             v.insert("cipher", querycipher);
//             // qDebug() << "iv is: " << queryiv << Qt::endl;
//             //  qDebug() << "key is: " << mykey << Qt::endl;
//             // qDebug() << "cipher is: " << querycipher << Qt::endl;
//             //  qDebug() << "password is: " << mypassword << Qt::endl;

//             bool feedback = mypayload->decryptpass(mypassword.toStdString(),v);

//             if(feedback) {


//                 qx::QxRestApi dataapi;
//                 QRandomGenerator64 *gen = QRandomGenerator64::system();
//                 gen->securelySeeded();
//                 long long discardx = gen->bounded(1000000000);
//                 // qDebug() << "my token  is: " << discardx << Qt::endl;
//                 QJsonDocument mydoc = QJsonDocument::fromJson(strPayload.toUtf8());
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "query");
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "relations");
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("cryptocenter"));
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "columns[0][0]", "token");
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.token", QJsonValue(discardx));
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.ids_id");
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.password");
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.subscription");
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.username");
//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.cryptocenter_id", QJsonValue(theid));
//                 QString mycryptojson = mydoc.toJson(QJsonDocument::JsonFormat::Indented);
//                 //  qDebug() << "my mycryptojson is: " << qPrintable(mycryptojson) << Qt::endl;
//                 QString resultcrypto = dataapi.processRequest(mycryptojson);
//                 Q_UNUSED(resultcrypto)
//                 // qDebug() << "resultcrypto for token in relation cryptocenter is: " << resultcrypto  << Qt::endl;
//                 QJsonWebToken m_jwtObj;
//                 QDateTime mycurrent = QDateTime::currentDateTime();
//                 QString mydate = mycurrent.toString(Qt::ISODate);
//                 mypayload->processRequest(resultcrypto);
//                 QJsonObject myregpayload = mypayload->m_requestJson.toObject();
//                 QJsonValue myregvalue = myregpayload.value("data");
//                 QJsonObject myregrequest = myregvalue.toObject();
//                 QString myids_id =  myregrequest.value("cryptocenter_id").toString();
//                 m_jwtObj.removeAll();
//                 m_jwtObj.appendClaim(myusername,  QString::number(discardx));
//                 m_jwtObj.setAlgorithmStr("HSSha3_512");
//                 m_jwtObj.setSecret(strSecret);
//                 QJsonArray newarr;
//                 QJsonObject object;
//                 object.insert("token", m_jwtObj.getToken());
//                 object.insert("ids_id", myids_id);
//                 object.insert("date", QString::fromStdString(mydate.toStdString()));
//                 newarr << object;
//                 QJsonDocument responsedoc(newarr);
//                 //  QString myresponsedocjson = responsedoc.toJson(QJsonDocument::JsonFormat::Indented);
//                 //   qDebug() << "myresponsedocjson is: " << qPrintable(myresponsedocjson) << Qt::endl;
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);
//                 errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                          << myrequest.value("created_at").toString().toStdString() << " ." ;
//                 sccentry << " Logging in of user " << myrequest.value("username").toString().toStdString() <<" at "
//                          << myrequest.value("created_at").toString().toStdString() << " was successful." ;





//             }

//             else {

//                 const std::string ErrorTemplate = "wrongpassword";
//                 QString result = (QString::fromStdString(ErrorTemplate));
//                 response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//                 response.status() = 403;
//                 response.data() = result.toUtf8();
//                 sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                          << myrequest.value("created_at").toString().toStdString() << " ." ;
//                 errentry << " Logging in of user " << myrequest.value("username").toString().toStdString() <<" at "
//                          << myrequest.value("created_at").toString().toStdString() << " was not successful since a wrong password was supplied." ;


//             }


//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " Logging in of user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;



//         }




//     });


//     // End login user



//     // Start Retrieve password for a  user

//     http.dispatch("POST", "/deliva/ids/hint", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errhint.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS +"/scchint.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString queryparam =  request.params().value("reason");
//         // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         //   QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         QString strPayload = QString::fromUtf8(request.data());
//         //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myusername =  myrequest.value("username").toString();
//         QString myhint =  myrequest.value("hint").toString();
//         long long token1 = queryparam.toLongLong();
//         QString slash1 = + "\"" + myusername + "\"";
//         QString slash2 = + "\"" + myhint + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         //  QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         QString query2 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + ",\"hint\" : " +
//                          slash2 +" }";
//         //   QString query1 = "{ \"username\" : " + slash1 + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
//         // qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         //  modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query2);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         // qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         Q_UNUSED(resultcrypto)
//         //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         // QString queryusername =  myrequestfromquery.value("username").toString();
//         QString queryiv =  myrequestfromquery.value("iv").toString();
//         QString querycipher =  myrequestfromquery.value("cipher").toString();
//         QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         QString mykey = "none";
//         qint64 querypolicy =  myrequestfromquery.value("policy").toInt();

//         if(querypolicy==0) {

//             mykey = myrequestfromquery.value("key").toString();

//         }

//         else {

//             mykey = myrequest.value("key").toString();

//         }

//         QVariantMap v;
//         v.insert("key", mykey);
//         v.insert("iv", queryiv);
//         v.insert("cipher", querycipher);
//         // qDebug() << "iv is: " << queryiv << Qt::endl;
//         //  qDebug() << "key is: " << mykey << Qt::endl;
//         // qDebug() << "cipher is: " << querycipher << Qt::endl;

//         QString mypasscred = QString::fromStdString(mypayload->recoverpass(v));

//         if(!(mypasscred=="")) {



//             QJsonArray newarr;
//             QJsonObject object;
//             object.insert("password", mypasscred);
//             object.insert("ids_id", theid);
//             newarr << object;
//             QJsonDocument responsedoc(newarr);
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = responsedoc.toJson(QJsonDocument::Compact);
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " Retrieving password for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was successful." ;





//         }

//         else {

//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " Retrieving password for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since"
//                                                                                   " access was denied." ;


//         }






//     });


//     // End Retrieve password for a  user




//     // Start basic updates

//     http.dispatch("POST", "/deliva/universal/basicupdates", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errbasicupdates.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccbasicupdates.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString queryparam =  request.params().value("reason");
//         // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         //   QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         QString strPayload = QString::fromUtf8(request.data());
//         //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myaction =  mydearpayload.value("action").toString();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myusername =  myrequest.value("username").toString();
//         long long token1 = queryparam.toLongLong();
//         QString slash1 = + "\"" + myusername + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
//         // qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         Q_UNUSED(resultcrypto)
//         //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::endl;



//         if((querytoken==token1)) {

//             QJsonDocument operationdoc = QJsonDocument::fromJson(strPayload.toUtf8());

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="cryptocenter") && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist"  ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if(myaction=="update") {

//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "query.sql");
//             }

//             if(myaction=="fetch_by_id") {

//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "query.sql");
//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.username");

//             }

//             if(myaction=="fetch_by_query") {

//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data");

//             }


//             QString operationjson = operationdoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "operationjson is: " << qPrintable(operationjson) << Qt::endl;
//             qx::QxRestApi dataapi;
//             QString resultdata = dataapi.processRequest(operationjson);
//             //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
//             /*  qDebug() << "Json Doc for resultdata id_ids is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented))
//                   << Qt::endl; */

//             if(myentity=="ids") {

//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "entity", QJsonValue("cryptocenter"));
//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "data.ids_id");
//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "data.cryptocenter_id", QJsonValue(theid));
//                 QString ccjson = operationdoc .toJson(QJsonDocument::JsonFormat::Indented);
//                 //   qDebug() << "ccjson is: " << qPrintable(ccjson) << Qt::endl;
//                 QString resultdatacc = dataapi.processRequest(ccjson);
//                 //  QJsonDocument myccdoc = QJsonDocument::fromJson(resultdatacc.toUtf8());
//                 /* qDebug() << "Json Doc for resultdatacc is: " << qPrintable(myccdoc.toJson(QJsonDocument::JsonFormat::Indented))
//                    << Qt::endl; */
//                 Q_UNUSED(resultdatacc)

//             }

//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = resultdata.toUtf8();
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " basic updates for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was successful." ;








//         }

//         else {

//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " basic updates for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


//         }






//     });


//     // End basic updates



//     // Start password edit

//     http.dispatch("POST", "/deliva/ids/secure", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errpasswordupdate.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccpasswordupdate.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString queryparam =  request.params().value("reason");
//         // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         //   QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         QString strPayload = QString::fromUtf8(request.data());
//         //  QJsonObject myobj = mypayload->ObjectFromString(strPayload);
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myaction =  mydearpayload.value("action").toString();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myusername =  myrequest.value("username").toString();
//         QString mypassword =  myrequest.value("password").toString();
//         long long token1 = queryparam.toLongLong();
//         QString slash1 = + "\"" + myusername + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
//         // qDebug() << "userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         Q_UNUSED(resultcrypto)
//         //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         qint64 mypolicy = myrequestfromquery.value("policy").toString().toInt();



//         if((querytoken==token1)) {

//             QJsonDocument operationdoc = QJsonDocument::fromJson(strPayload.toUtf8());

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("nullandvoid"));

//             }


//             modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "query");
//             //  QString operationjson = operationdoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "operationjson is: " << qPrintable(operationjson) << Qt::endl;
//             qx::QxRestApi dataapi;
//             QVariantMap mysecurity = modifyMyJsonToolsuser->encryptpass(mypassword.toStdString());
//             QString mycryptokey = mysecurity.value("key").toString();
//             modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "entity", QJsonValue("cryptocenter"));
//             modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "query.entity", QJsonValue("cryptocenter"));

//             if(mypolicy==0) {


//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.key", QJsonValue(mycryptokey));

//             }

//             else {


//                 modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.key", QJsonValue("policyretain"));


//             }

//             QStringList m_destlist;
//             m_destlist << "iv";
//             m_destlist << "key";
//             m_destlist << "cipher";
//             QJsonArray m_array = QJsonArray::fromStringList(m_destlist);
//             //   qDebug() << "m_array is: " << m_array << Qt::endl;
//             modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "columns");
//             modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "columns", m_array);
//             modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.iv",  QJsonValue(mysecurity.value("iv").toString()));
//             modifyMyJsonToolsuser->modifyJsonValue(operationdoc, "data.cipher",  QJsonValue(mysecurity.value("cipher").toString()));
//             modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "data.ids_id");
//             modifyMyJsonToolsuser->modifyJsonValue(operationdoc , "data.cryptocenter_id", QJsonValue(theid));
//             QString ccjson = operationdoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "ccjson is: " << qPrintable(ccjson) << Qt::endl;
//             QString resultdatacc = dataapi.processRequest(ccjson);
//             //  QJsonDocument myccdoc = QJsonDocument::fromJson(resultdatacc.toUtf8());
//             /* qDebug() << "Json Doc for resultdatacc is: " << qPrintable(myccdoc.toJson(QJsonDocument::JsonFormat::Indented))
//                    << Qt::endl; */
//             Q_UNUSED(resultdatacc)
//             mypayload->processRequest(resultdatacc);
//             QJsonObject myregpayload = mypayload->m_requestJson.toObject();
//             QJsonValue myregvalue = myregpayload.value("data");
//             QJsonObject myregrequest = myregvalue.toObject();
//             QString myids_id =  myregrequest.value("cryptocenter_id").toString();
//             QJsonArray newarr;
//             QJsonObject object;

//             if(mypolicy==1) {

//                 object.insert("key", mycryptokey);

//             }



//             object.insert("ids_id", myids_id);
//             newarr << object;
//             QJsonObject obj;
//             obj [ "data" ] = newarr;
//             QJsonDocument responsedoc(obj);
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = responsedoc.toJson(QJsonDocument::Compact);
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " password edit for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString()<< " was successful." ;







//         }

//         else {

//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " password edit for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


//         }






//     });


//     // End password edit





//     // Start get user details


//     http.dispatch("POST", "/deliva/ids/user", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         //  QString queryparam =  request.params().value("reason");
//         //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         // QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/erruserdetails.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccuserdetails.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString strPayload = QString::fromUtf8(request.data());
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myaction =  mydearpayload.value("action").toString();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString mytoken =  request.params().value("reason");
//         QString myusername =  myrequest.value("username").toString();
//         // qDebug() << "mytoken is: " << mytoken << Qt::endl;
//         long long token1 = mytoken.toLongLong();
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         // QString query1 = "{ \"username\" : " + slash1 + " }";
//         // QString query1 = "{ \"token\" : " + QString::number(token1) + " }";
//         // qDebug() << "query1 is: " << query1 << Qt::endl;
//         QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //qDebug() << "myfinaljson after modification is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         // QString queryresults =  myrequestfromquery.value("username").toString();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::endl;



//         if((querytoken==token1)) {



//             qx::QxRestApi dataapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "myusagejson in ids is: " << qPrintable(myusagejson) << Qt::endl;
//             QString resultdata = dataapi.processRequest(myusagejson);
//             //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
//             //  qDebug() << "Json Doc for resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//             mypayload->processRequest(resultdata);
//             QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
//             QJsonValue myregdata = mydearpayloaddata.value("data");
//             QJsonArray myqueryarr = myregdata.toArray();
//             QJsonDocument responsedoc(myqueryarr);
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = responsedoc.toJson(QJsonDocument::Compact);
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " fetch details for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " fetch details for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


//         }



//     });


//     // End get user details


//     // Start update subscription


//     http.dispatch("POST", "/deliva/ids/subscribe", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errsubscribe.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccsubscribe.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString queryparam =  request.params().value("reason");
//         // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         QString strPayload = QString::fromUtf8(request.data());
//         QString mytoken = queryparam;
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myaction =  mydearpayload.value("action").toString();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myauth =  myrequest.value("auth").toString();
//         Q_UNUSED(myauth)
//         QString myusername =  myrequest.value("username").toString();
//         QString myidids = myrequest.value("ids_id").toString();
//         qint64 mysub =  myrequest.value("subscription").toInt();
//         // qDebug() << "myauth is: " << myauth << Qt::endl;
//         qDebug() << "myusername is: " << myusername << Qt::endl;
//         // qDebug() << "mysub is: " << mysub << Qt::endl;
//         // qDebug() << "mytoken is: " << mytoken << Qt::endl;
//         long long token1 = mytoken.toLongLong();
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         // QString query1 = "{ \"username\" : " + slash1 + " }";
//         // QString query1 = "{ \"token\" : " + QString::number(token1) + " }";
//         // qDebug() << "query1 is: " << query1 << Qt::endl;
//         QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         qDebug() << "myfinaljson in subscription update is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         Q_UNUSED(resultcrypto)
//         //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         //QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::endl;



//         if((querytoken==token1)) {

//             qx::QxRestApi dataapi;

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("nullandvoid"));

//             }

//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "query");
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.auth");
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.username");
//             QString mydocjson = mydoc.toJson(QJsonDocument::JsonFormat::Indented);
//             // qDebug() << "my mydocjson  is: " << qPrintable(mydocjson) << Qt::endl;
//             QString resultdata = dataapi.processRequest(mydocjson);
//             Q_UNUSED(resultdata)
//             //  qDebug() << "resultdata for subscription in relation ids is: " << resultdata  << Qt::endl;
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("cryptocenter"));
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.ids_id");
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.cryptocenter_id", QJsonValue(myidids));
//             QString mycryptojson = mydoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "my mycryptojson is: " << qPrintable(mycryptojson) << Qt::endl;
//             QString resultcrypto = dataapi.processRequest(mycryptojson);
//             Q_UNUSED(resultcrypto)
//             //   qDebug() << "resultcrypto for subscription in relation cryptocenter is: " << resultcrypto  << Qt::endl;
//             const std::string ErrorTemplate = "success";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 200;
//             response.data() = result.toUtf8();
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " subscription update for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString()<< " was successful." ;



//         }

//         else {


//             const std::string SuccTemplate = "denied";
//             QString result = (QString::fromStdString(SuccTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " subscription update for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString()<< " was not successful since access was denied." ;


//         }




//     });

//     // End update subscription


//     // Start signout user

//     http.dispatch("POST", "/deliva/ids/logout", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errlogout.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/scclogout.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString queryparam =  request.params().value("reason");
//         // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         QString strPayload = QString::fromUtf8(request.data());
//         QString mytoken = queryparam;
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myaction =  mydearpayload.value("action").toString();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myauth =  myrequest.value("auth").toString();
//         Q_UNUSED(myauth)
//         QString myusername =  myrequest.value("username").toString();
//         QString myidids = myrequest.value("ids_id").toString();
//         qint64 mysub =  myrequest.value("subscription").toInt();
//         // qDebug() << "myauth is: " << myauth << Qt::endl;
//         // qDebug() << "myusername is: " << myusername << Qt::endl;
//         // qDebug() << "mysub is: " << mysub << Qt::endl;
//         // qDebug() << "mytoken is: " << mytoken << Qt::endl;
//         long long token1 = mytoken.toLongLong();
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         // QString query1 = "{ \"username\" : " + slash1 + " }";
//         // QString query1 = "{ \"token\" : " + QString::number(token1) + " }";
//         // qDebug() << "query1 is: " << query1 << Qt::endl;
//         QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         // qDebug() << "myfinaljson is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         Q_UNUSED(resultcrypto)
//         //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::endl;



//         if((querytoken==token1)) {

//             qx::QxRestApi dataapi;
//             QRandomGenerator64 *gen = QRandomGenerator64::system();
//             gen->securelySeeded();
//             long long discardx = gen->bounded(1000000000);
//             // qDebug() << "my token  is: " << discardx << Qt::endl;

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="ids" || myentity=="cryptocenter"  ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist"  ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("nullandvoid"));

//             }


//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.logout");
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "query");
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "entity", QJsonValue("cryptocenter"));
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "columns[0][0]", "token");
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.token", QJsonValue(discardx));
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.ids_id");
//             modifyMyJsonToolsuser->modifyJsonValue(mydoc, "data.cryptocenter_id", QJsonValue(myidids));
//             QString mycryptojson = mydoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //   qDebug() << "my mycryptojson is: " << qPrintable(mycryptojson) << Qt::endl;
//             QString resultcrypto = dataapi.processRequest(mycryptojson);
//             Q_UNUSED(resultcrypto)
//             // qDebug() << "resultcrypto for token in relation cryptocenter is: " << resultcrypto  << Qt::endl;
//             const std::string TokenTemplate = QString::number(discardx).toStdString();
//             QString result = (QString::fromStdString(TokenTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 200;
//             response.data() = result.toUtf8();
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " logout operation for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was successful." ;



//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " logout operation for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


//         }


//     });


//     // End signout user



//     // Start create new category

//     http.dispatch("POST", "/deliva/category/new", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errnewcategory.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccnewcategory.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         // QJsonDocument originaldoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "originaldoc is: " << qPrintable(originaldoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//         // QString myrequestdoc = QString::fromUtf8(request.data());
//         QString queryparam =  request.params().value("reason");
//         // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         QString strPayload = QString::fromUtf8(request.data());
//         QString mytoken = queryparam;
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myaction =  mydearpayload.value("action").toString();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myusername =  myrequest.value("username").toString();
//         //   QString myidids = myrequest.value("ids_id").toString();
//         qint64 mysub =  myrequest.value("subscription").toInt();
//         // qDebug() << "myidids is: " << myidids << Qt::endl;
//         // qDebug() << "myusername is: " << myusername << Qt::endl;
//         // qDebug() << "mysub is: " << mysub << Qt::endl;
//         // qDebug() << "mytoken is: " << mytoken << Qt::endl;
//         long long token1 = mytoken.toLongLong();
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString slash2 = + "\"" + myrequest.value("name").toString() + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         QString query2 = "{ \"name\" : " + slash2 + ",\"username\" : " + slash1 + " }";
//         // QString query1 = "{ \"token\" : " + QString::number(token1) + " }";
//         // qDebug() << "query1 is: " << query1 << Qt::endl;
//         QJsonValue myvaluequery = mydearpayload.value("query");
//         QJsonObject myrequestquery = myvaluequery.toObject();
//         //  QString myentityquery =  myrequestquery.value("entity").toString();
//         // qDebug() << "myentityquery is: " << myentityquery << Qt::endl;
//         QJsonDocument userdoc(myrequestquery);
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "myfinaljson in category crypto is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         Q_UNUSED(resultcrypto)
//         // qDebug() << "result after query in crypto category is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         //  QString queryresults =  myrequestfromquery.value("username").toString();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::end
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query2);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("categories"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         QString mycatjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //   qDebug() << "mycatjson in categories is: " << qPrintable(mycatjson) << Qt::endl;
//         qx::QxRestApi catapi;
//         QString resultcat = catapi.processRequest(mycatjson);
//         //   qDebug() << "result after query in categories is: " << resultcat << Qt::endl;
//         mypayload->processRequest(resultcat);
//         QJsonObject mydearpayloadcatresults = mypayload->m_requestJson.toObject();
//         QJsonValue mycatvalue = mydearpayloadcatresults.value("data");
//         QJsonArray myrequestcatfromqueryarr = mycatvalue.toArray();
//         QJsonValue datacatFirstarr = myrequestcatfromqueryarr.first();
//         QJsonObject myrequestcatfromquery = datacatFirstarr.toObject();
//         QString querycatresults =  myrequestcatfromquery.value("name").toString();
//         //   qDebug() << "querycatresults in categories is: " << querycatresults << Qt::endl;

//         if(querytoken==token1 && !(querycatresults==myrequest.value("name").toString())) {




//             qx::QxRestApi loadapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"   || myaction =="update" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }


//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query");
//             QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "myusagejson in category is: " << qPrintable(myusagejson) << Qt::endl;
//             QString resultreg = loadapi.processRequest(myusagejson);
//             // qDebug() << "result after inserting new category is: " << resultreg << Qt::endl;
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = resultreg.toUtf8();
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " creation of new category for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " creation of new category for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;



//         }



//     });

//     // End create new category


//     // Start create new item

//     http.dispatch("POST", "/deliva/menu/new", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/erritemnew.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccitemnew.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QByteArray utfString = request.data();
//         const long mysize  =  request.header("Content-Length").toLong();
//         std::string multipartContent = request.header("Content-Type").toStdString();
//         std::string jsondata ="";
//         jsondata = mypayload->doupload(utfString,mypayload->UPLOAD_TEMPDIRIDS,
//                                        mypayload->UPLOAD_DIRIDS,mysize,multipartContent,false);

//         //   QJsonDocument originaldoc = QJsonDocument::fromJson(QString::fromStdString(jsondata).toUtf8());
//         //  qDebug() << "originaldoc is: " << qPrintable(originaldoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//         // QString myrequestdoc = QString::fromUtf8(QString::fromStdString(jsondata).toUtf8());
//         QString queryparam =  request.params().value("reason");
//         // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         QString strPayload = QString::fromStdString(jsondata);
//         QString mytoken = queryparam;
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QString myaction =  mydearpayload.value("action").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myusername =  myrequest.value("username").toString();
//         long long token1 = mytoken.toLongLong();
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString slash2 = + "\"" + myrequest.value("name").toString() + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         QString query2 = "{ \"name\" : " + slash2 + ",\"username\" : " + slash1 + " }";
//         // QString query1 = "{ \"token\" : " + QString::number(token1) + " }";
//         // qDebug() << "query1 is: " << query1 << Qt::endl;
//         QJsonValue myvaluequery = mydearpayload.value("query");
//         QJsonObject myrequestquery = myvaluequery.toObject();
//         //  QString myentityquery =  myrequestquery.value("entity").toString();
//         // qDebug() << "myentityquery is: " << myentityquery << Qt::endl;
//         QJsonDocument userdoc(myrequestquery);
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         // qDebug() << "myfinaljson in items crypto is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         Q_UNUSED(resultcrypto)
//         //  qDebug() << "result after query in crypto items is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         // QString queryresults =  myrequestfromquery.value("username").toString();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         // QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::endl;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query2);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("items"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         QString myitemsjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "myitemsjson in items is: " << qPrintable(myitemsjson) << Qt::endl;
//         qx::QxRestApi itemsapi;
//         QString resultitems = itemsapi.processRequest(myitemsjson);
//         //  qDebug() << "result after query in items is: " << resultitems << Qt::endl;
//         mypayload->processRequest(resultitems);
//         QJsonObject mydearpayloaditemsresults = mypayload->m_requestJson.toObject();
//         QJsonValue myitemsvalue = mydearpayloaditemsresults.value("data");
//         QJsonArray myrequestitemsfromqueryarr = myitemsvalue.toArray();
//         QJsonValue dataitemsFirstarr = myrequestitemsfromqueryarr.first();
//         QJsonObject myrequestitemsfromquery = dataitemsFirstarr.toObject();
//         QString queryitemsresults =  myrequestitemsfromquery.value("name").toString();
//         //  qDebug() << "queryitemsresults in items is: " << queryitemsresults << Qt::endl;

//         if(querytoken==token1 && !(queryitemsresults==myrequest.value("name").toString())) {


//             jsondata = mypayload->doupload(utfString,mypayload->UPLOAD_TEMPDIRIDS,
//                                            mypayload->UPLOAD_DIRIDS,mysize,multipartContent,true,false,"");


//             qx::QxRestApi loadapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(QString::fromStdString(jsondata).toUtf8());

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="update" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }


//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query");
//             QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "myusagejson in item is: " << qPrintable(myusagejson) << Qt::endl;
//             QString resultitemz = loadapi.processRequest(myusagejson);
//             //  qDebug() << "result after inserting new item is: " << resultitemz<< Qt::endl;
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = resultitemz.toUtf8();
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " creation of new item for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was successful." ;





//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " creation of new item for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


//         }



//     });

//     // End create new item

//     // Start fetch specific

//     http.dispatch("POST", "/deliva/universal/fetchspecific", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         //  QString queryparam =  request.params().value("reason");
//         //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         //  QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchspecific.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchspecific.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString strPayload = QString::fromUtf8(request.data());
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QString myaction =  mydearpayload.value("action").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString mytoken =  request.params().value("reason");
//         QString myusername =  myrequest.value("username").toString();
//         QString mytarget =  mydearpayload.value("target").toString();
//         //  qDebug() << "mytarget is: " << mytarget << Qt::endl;
//         // qDebug() << "mytoken is: " << mytoken << Qt::endl;
//         long long token1 = mytoken.toLongLong();
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         // QString query1 = "{ \"username\" : " + slash1 + " }";
//         // QString query1 = "{ \"token\" : " + QString::number(token1) + " }";
//         // qDebug() << "query1 is: " << query1 << Qt::endl;
//         QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //qDebug() << "myfinaljson after modification is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::endl;



//         if((querytoken==token1)) {

//             qx::QxRestApi dataapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }


//             QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
//             //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
//             //  qDebug() << "Json Doc for resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//             mypayload->processRequest(resultdata);
//             QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
//             QJsonValue myregdata = mydearpayloaddata.value("data");
//             QJsonArray myqueryarr = myregdata.toArray();
//             QJsonValue dataFirstarr = myqueryarr.first();
//             QJsonObject myrequestfromquery = dataFirstarr.toObject();

//             if(mytarget == "categories") {

//                 QJsonValue myregdatacat = myrequestfromquery.value("list_catids");
//                 QJsonArray myqueryarrcat = myregdatacat.toArray();
//                 QJsonDocument responsedoc(myqueryarrcat);
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);

//             }

//             else if (mytarget == "items") {


//                 QJsonValue myregdataitem = myrequestfromquery.value("list_itemsids");
//                 QJsonArray myqueryarritem = myregdataitem.toArray();
//                 QJsonDocument responsedoc(myqueryarritem);
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);


//             }

//             else if (mytarget == "delivery") {

//                 QJsonValue myregdatadeli = myrequestfromquery.value("list_deliveryids");
//                 QJsonArray myqueryarrdeli = myregdatadeli.toArray();
//                 QJsonDocument responsedoc(myqueryarrdeli);
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);



//             }

//             else if (mytarget == "logistics") {

//                 QJsonValue myregdatalogi = myrequestfromquery.value("list_logisticsids");
//                 QJsonArray myqueryarrlogi = myregdatalogi.toArray();
//                 QJsonDocument responsedoc(myqueryarrlogi);
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);



//             }

//             else if (mytarget == "orders") {


//                 QJsonValue myregdataord = myrequestfromquery.value("list_ordersids");
//                 QJsonArray myqueryarrord = myregdataord.toArray();
//                 QJsonDocument responsedoc(myqueryarrord);
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);


//             }



//             else if (mytarget == "locations") {

//                 QJsonValue myregdataloc = myrequestfromquery.value("list_locationsids");
//                 QJsonArray myqueryarrloc= myregdataloc.toArray();
//                 QJsonDocument responsedoc(myqueryarrloc);
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);


//             }

//             else if (mytarget == "metrics") {


//                 QJsonValue myregdatamet = myrequestfromquery.value("list_metricsids");
//                 QJsonArray myqueryarrmet= myregdatamet.toArray();
//                 QJsonDocument responsedoc(myqueryarrmet);
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);



//             }

//             else {


//                 QJsonDocument responsedoc(myqueryarr);
//                 response.headers().insert("Content-Type", "application/json; charset=utf-8");
//                 response.data() = responsedoc.toJson(QJsonDocument::Compact);


//             }




//             // QString queryresults =  myrequestfromquery.value("username").toString();
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " fetch specific data for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was successful." ;





//         }

//         else {

//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " fetch specific data for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;





//         }


//     });


//     // End fetch specific


//     // Start update basic avatars

//     http.dispatch("POST", "/deliva/universal/updatebasicavatars", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errupdatebasicavatars.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccupdatebasicavatars.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QByteArray utfString = request.data();
//         const long mysize  =  request.header("Content-Length").toLong();
//         std::string multipartContent = request.header("Content-Type").toStdString();
//         std::string jsondata ="";
//         jsondata = mypayload->doupload(utfString,mypayload->UPLOAD_TEMPDIRIDS,
//                                        mypayload->UPLOAD_DIRIDS,mysize,multipartContent,false);
//         //  QJsonDocument originaldoc = QJsonDocument::fromJson(QString::fromStdString(jsondata).toUtf8());
//         //   qDebug() << "originaldoc is: " << qPrintable(originaldoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//         // QString myrequestdoc = QString::fromUtf8(QString::fromStdString(jsondata).toUtf8());
//         QString queryparam =  request.params().value("reason");
//         // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         QString strPayload = QString::fromStdString(jsondata);
//         QString mytoken = queryparam;
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myfield =  mydearpayload.value("field").toString();
//         QString myavatarfield =  mydearpayload.value("avatarfield").toString();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QString myaction =  mydearpayload.value("action").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myusername =  myrequest.value("username").toString();
//         long long token1 = mytoken.toLongLong();
//         QString dearfield = + "\"" + myfield + "\"";
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString slash2 = + "\"" + myrequest.value(myfield).toString() + "\"";
//         //   QString slash3 = + "\"" + myavatarfield + "\"";
//         //  QString slash4 = + "\"" + myrequest.value(myavatarfield).toString() + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         QString query2 = "{ " + dearfield + " : " + slash2 + " }";
//         //  QString query2 = "{ \"name\" : " + slash2 + " }";
//         //   QString query2 = "{ " + dearfield + " : " + slash2 + "," + slash3 + " : " + slash4 + "}";
//         // QString query1 = "{ \"token\" : " + QString::number(token1) + " }";
//         // qDebug() << "query1 is: " << query1 << Qt::endl;
//         //   qDebug() << "query2 is: " << query2 << Qt::endl;
//         QJsonValue myvaluequery = mydearpayload.value("query");
//         QJsonObject myrequestquery = myvaluequery.toObject();
//         //  QString myentityquery =  myrequestquery.value("entity").toString();
//         // qDebug() << "myentityquery is: " << myentityquery << Qt::endl;
//         QJsonDocument userdoc(myrequestquery);
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         // qDebug() << "myfinaljson in update basic avatars crypto is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         Q_UNUSED(resultcrypto)
//         //  qDebug() << "result after query in crypto update basic avatars  is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         // QString queryresults =  myrequestfromquery.value("username").toString();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         // QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::endl;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query2);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue(myentity));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         QString myitemsjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "myitemsjson in update basic avatars  is: " << qPrintable(myitemsjson) << Qt::endl;
//         qx::QxRestApi itemsapi;
//         QString resultitems = itemsapi.processRequest(myitemsjson);
//         //  qDebug() << "result after query in update basic avatars  is: " << resultbasicavatars << Qt::endl;
//         mypayload->processRequest(resultitems);
//         QJsonObject mydearpayloaditemsresults = mypayload->m_requestJson.toObject();
//         QJsonValue myitemsvalue = mydearpayloaditemsresults.value("data");
//         QJsonArray myrequestitemsfromqueryarr = myitemsvalue.toArray();
//         QJsonValue dataitemsFirstarr = myrequestitemsfromqueryarr.first();
//         QJsonObject myrequestitemsfromquery = dataitemsFirstarr.toObject();
//         QString queryitemsresults =  myrequestitemsfromquery.value(myfield).toString();
//         QString queryavatarnameresults =  myrequestitemsfromquery.value(myavatarfield).toString();
//         //  qDebug() << "queryitemsresults in update basic avatars  is: " << queryitemsresults << Qt::endl;

//         if(querytoken==token1 && (queryitemsresults==myrequest.value(myfield).toString())) {

//             jsondata = mypayload->doupload(utfString,mypayload->UPLOAD_TEMPDIRIDS,
//                                            mypayload->UPLOAD_DIRIDS,mysize,multipartContent,true,true,
//                                            queryavatarnameresults.toStdString());
//             qx::QxRestApi loadapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(QString::fromStdString(jsondata).toUtf8());

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }


//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query");
//             QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "myusagejson in update basic avatars  is: " << qPrintable(myusagejson) << Qt::endl;
//             QString resultitemz = loadapi.processRequest(myusagejson);
//             //  qDebug() << "result after update basic avatars  is: " << resultitemz<< Qt::endl;
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = resultitemz.toUtf8();
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " updating basic avatars for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was successful." ;





//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " updating basic avatars for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


//         }


//     });

//     // End update basic avatars


//     // Start other operations

//     http.dispatch("POST", "/deliva/universal/otheroperations", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errotheroperations.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccotheroperations.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString strPayload = QString::fromUtf8(request.data());
//         //  QJsonDocument originaldoc = QJsonDocument::fromJson(strPayload.toUtf8());
//         //  qDebug() << "originaldoc is: " << qPrintable(originaldoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//         //  QString myrequestdoc = QString::fromUtf8(strPayload.toUtf8());
//         QString queryparam =  request.params().value("reason");
//         // qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         QString mytoken = queryparam;
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myfield =  mydearpayload.value("field").toString();
//         QString myavatarfield =  mydearpayload.value("avatarfield").toString();
//         //  qDebug() <<"avatar field is: " << myavatarfield  << Qt::endl;
//         /*   if(myavatarfield=="" || myavatarfield.isEmpty() || myavatarfield.isNull()) {

//            myavatarfield="nullstr";

//         } */

//         if(myavatarfield=="") {

//             myavatarfield="nullstr";

//         }

//         bool deleteother =  mydearpayload.value("deleteother").toBool();
//         QString deleteotherfield =  mydearpayload.value("deleteotherfield").toString();
//         QString deleteotherentity =  mydearpayload.value("deleteotherentity").toString();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QString myaction =  mydearpayload.value("action").toString();
//         QJsonValue myvalue = mydearpayload.value("info");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myusername =  myrequest.value("username").toString();
//         long long token1 = mytoken.toLongLong();
//         QString dearfield = + "\"" + myfield + "\"";
//         QString otherfield = + "\"" + mydearpayload.value("otherfield").toString() + "\"";
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString slash2 = + "\"" + myrequest.value(myfield).toString() + "\"";
//         QString slash3 = + "\"" + myrequest.value(deleteotherfield).toString() + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         //  QString query2 = "{ \"name\" : " + slash2 + " }";
//         QString query2 = "{ " + dearfield + " : " + slash2 + " }";
//         QString query3 = "{ " + otherfield + " : " + slash3 + " }";
//         // QString query1 = "{ \"token\" : " + QString::number(token1) + " }";
//         // qDebug() << "query1 is: " << query1 << Qt::endl;
//         QJsonValue myvaluequery = mydearpayload.value("query");
//         QJsonObject myrequestquery = myvaluequery.toObject();
//         //  QString myentityquery =  myrequestquery.value("entity").toString();
//         // qDebug() << "myentityquery is: " << myentityquery << Qt::endl;
//         QJsonDocument userdoc(myrequestquery);
//         QJsonDocument myentityfunction = userdoc;
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         // qDebug() << "myfinaljson in other operations crypto is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         Q_UNUSED(resultcrypto)
//         //  qDebug() << "result after query in crypto other operations  is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         // QString queryresults =  myrequestfromquery.value("username").toString();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         // QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::endl;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query2);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue(myentity));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         QString myitemsjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         // qDebug() << "myitemsjson in other operations  is: " << qPrintable(myitemsjson) << Qt::endl;
//         qx::QxRestApi itemsapi;
//         QString resultitems = itemsapi.processRequest(myitemsjson);
//         //  qDebug() << "result after query in other operations  is: " << resultitems << Qt::endl;
//         mypayload->processRequest(resultitems);
//         QJsonObject mydearpayloaditemsresults = mypayload->m_requestJson.toObject();
//         QJsonValue myitemsvalue = mydearpayloaditemsresults.value("data");
//         QJsonArray myrequestitemsfromqueryarr = myitemsvalue.toArray();
//         QJsonValue dataitemsFirstarr = myrequestitemsfromqueryarr.first();
//         QJsonObject myrequestitemsfromquery = dataitemsFirstarr.toObject();
//         QString queryitemsresults =  myrequestitemsfromquery.value(myfield).toString();
//         QString queryitemsavatarname =  myrequestitemsfromquery.value(myavatarfield).toString();
//         QStringList m_deletemedia;
//         m_deletemedia << queryitemsavatarname;
//         qx::QxRestApi entityapi;
//         QJsonArray m_array = QJsonArray::fromStringList(m_deletemedia);
//         modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "action", QJsonValue("call_entity_function"));
//         modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "query");
//         modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "entity", QJsonValue(myentity));
//         modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "fct","deletemediaite");
//         modifyMyJsonToolsuser->modifyJsonValue(myentityfunction, "data.media", m_array);
//         QString myentityjson = myentityfunction.toJson(QJsonDocument::JsonFormat::Indented);
//         // qDebug() << "myentityjson in other operations  is: " << qPrintable(myentityjson) << Qt::endl;
//         QString entityfctresults = entityapi.processRequest(myentityjson);
//         Q_UNUSED(entityfctresults)
//         // qDebug() << "result after entityfctresults in other operations  is: " << entityfctresults << Qt::endl;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query3);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "columns");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue(deleteotherentity));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         QString myanotherjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "myanotherjson in other operations  is: " << qPrintable(myanotherjson) << Qt::endl;
//         QString resultanother = itemsapi.processRequest(myanotherjson);
//         mypayload->processRequest(resultanother);
//         QJsonObject mydearpayloadresultanother = mypayload->m_requestJson.toObject();
//         QJsonValue myresultanothervalue = mydearpayloadresultanother.value("data");
//         QJsonArray myrequestresultanotherfromqueryarr = myresultanothervalue.toArray();
//         QJsonValue dataresultanotherFirstarr = myrequestresultanotherfromqueryarr.first();
//         QJsonObject myrequestresultanotherfromquery = dataresultanotherFirstarr.toObject();
//         QString queryresultanotherresults =  myrequestresultanotherfromquery.value(deleteotherentity+"_id").toString();

//         if(querytoken==token1 && (queryitemsresults==myrequest.value(myfield).toString())) {

//             qx::QxRestApi loadapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(strPayload.toUtf8());

//             // if((myentity=="ids" || myentity=="cryptocenter"  ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myaction=="call_entity_function"  || myentity=="cryptocenter") && (myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }


//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query");
//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "info");
//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "deleteother");
//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "deleteotherentity");
//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "deleteotherfield");
//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "field");
//             QString myusagejson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
//             //  qDebug() << "myusagejson in other operations  is: " << qPrintable(myusagejson) << Qt::endl;
//             QString resultitemz = loadapi.processRequest(myusagejson);
//             //  qDebug() << "result after other operations  is: " << resultitemz<< Qt::endl;
//             if(deleteother) {


//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "data."+deleteotherentity+"_id",queryresultanotherresults);
//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "data."+myentity+"_id");
//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue(deleteotherentity));
//                 QString myusagedeleteotherjson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
//                 // qDebug() << "myusagedeleteotherjson in other operations  is: " << qPrintable(myusagedeleteotherjson) << Qt::endl;
//                 QString resultitemzdeleteother = loadapi.processRequest(myusagedeleteotherjson);
//                 Q_UNUSED(resultitemzdeleteother)
//                 //  qDebug() << "resultdeleteother after other operations  is: " << resultitemzdeleteother<< Qt::endl;

//             }
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = resultitemz.toUtf8();
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " other operations for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was successful." ;





//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " other operations for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


//         }


//     });

//     // End other operations

//     // Start fetch all details


//     http.dispatch("POST", "/deliva/generic/fetchall", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         //  QString queryparam =  request.params().value("reason");
//         //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         // QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc fetch all is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchall.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchall.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString strPayload = QString::fromUtf8(request.data());
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QString myaction =  mydearpayload.value("action").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString mytoken =  request.params().value("reason");
//         QString myusername =  myrequest.value("username").toString();
//         // qDebug() << "mytoken is: " << mytoken << Qt::endl;
//         long long token1 = mytoken.toLongLong();
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         //  QString slash2 = + "\"" + mytoken + "\"";
//         QString query1 = "{ \"username\" : " + slash1 + ",\"token\" : " + QString::number(token1) + " }";
//         QString query0 = "{ \"username\" : " + slash1 + " }";
//         // QString query1 = "{ \"token\" : " + QString::number(token1) + " }";
//         // qDebug() << "query1 is: " << query1 << Qt::endl;
//         QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query1);
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         // QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Compact);
//         QString myfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //qDebug() << "myfinaljson after modification is: " << qPrintable(myfinaljson) << Qt::endl;
//         qx::QxRestApi cryptoapi;
//         QString resultcrypto = cryptoapi.processRequest(myfinaljson);
//         //  qDebug() << "result after query in crypto is: " << resultcrypto << Qt::endl;
//         mypayload->processRequest(resultcrypto);
//         QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
//         QJsonValue myregvalue = mydearpayloadresults.value("data");
//         QJsonArray myrequestfromqueryarr = myregvalue.toArray();
//         QJsonValue dataFirstarr = myrequestfromqueryarr.first();
//         QJsonObject myrequestfromquery = dataFirstarr.toObject();
//         // QString queryresults =  myrequestfromquery.value("username").toString();
//         long long querytoken =  myrequestfromquery.value("token").toInt();
//         int myrole = myrequestfromquery.value("roles").toInt();
//         //  QString theid = myrequestfromquery.value("cryptocenter_id").toString();
//         //  qDebug() << "querytoken is: " << querytoken << Qt::endl;
//         //  qDebug() << "token1 is: " << token1 << Qt::endl;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query0);
//         QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
//         qx::QxRestApi cryptoapiadsupwebjson;
//         QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
//         // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
//         mypayload->processRequest(resultcryptoadsupwebjson);
//         QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
//         QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
//         QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
//         QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
//         QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
//         // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
//         //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
//         qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
//         //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





//         if((querytoken==token1)) {



//             qx::QxRestApi dataapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());



//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }



//             // qDebug() << "Usage JSON for fetch all details is: " << qPrintable(usagedoc.toJson(QJsonDocument::Indented)) << Qt::endl;
//             QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
//             //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
//             //  qDebug() << "Json Doc for fetchall resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//             mypayload->processRequest(resultdata);
//             QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
//             QJsonValue myregdata = mydearpayloaddata.value("data");
//             QJsonArray myqueryarr = myregdata.toArray();
//             QJsonDocument responsedoc(myqueryarr);
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = responsedoc.toJson(QJsonDocument::Compact);
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " fetch all details by user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " fetch all details by user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since access was denied." ;


//         }



//     });


//     // End fetch all details


//     // Start admin operations

//     http.dispatch("POST", "/deliva/universal/adminoperations", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         //  QString queryparam =  request.params().value("reason");
//         //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         //  QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/erradminoperations.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccadminoperations.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString strPayload = QString::fromUtf8(request.data());
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QString myaction =  mydearpayload.value("action").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myreason =  request.params().value("reason");
//         QString myusername =  ((myrequest.value("username").toString()=="" || myrequest.value("username").toString().isNull())  ?  myrequest.value("username").toString() : myreason) ;
//         // qDebug() << "myreason is: " << myreason << Qt::endl;
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString query0 = "{ \"username\" : " + slash1 + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         qx::QxRestApi cryptoapi;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query0);
//         QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
//         qx::QxRestApi cryptoapiadsupwebjson;
//         QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
//         // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
//         mypayload->processRequest(resultcryptoadsupwebjson);
//         QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
//         QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
//         QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
//         QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
//         QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
//         // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
//         //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
//         qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
//         //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





//         if((adsupwebjsonroles>=2)) {



//             qx::QxRestApi dataapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());
//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {

//             // if((myentity=="ids" || myentity=="cryptocenter" ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }


//             QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
//             // QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
//             //  qDebug() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//             mypayload->processRequest(resultdata);
//             QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
//             QJsonValue myregdata = mydearpayloaddata.value("data");
//             QJsonArray myqueryarr = myregdata.toArray();
//             QJsonDocument responsedoc(myqueryarr);
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = responsedoc.toJson(QJsonDocument::Compact);
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " Admin operation by admin " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " Admin operation by admin " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since the user is not admin." ;


//         }



//     });


//     // End admin operations


//     // Start core super power operations

//     http.dispatch("POST", "/deliva/universal/coreoperations", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         //  QString queryparam =  request.params().value("reason");
//         //   qDebug() << "query param for ids is: " << queryparam << Qt::endl;
//         //  QJsonDocument mydoc = QJsonDocument::fromJson(request.data());
//         //  qDebug() << "Json Doc is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errcoreoperations.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/scccoreoperations.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString strPayload = QString::fromUtf8(request.data());
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         // QString myentity =  mydearpayload.value("entity").toString();
//         // QString myaction =  mydearpayload.value("action").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myreason =  request.params().value("reason");
//         QString myusername =  ((myrequest.value("username").toString()=="" || myrequest.value("username").toString().isNull())  ?  myrequest.value("username").toString() : myreason) ;
//         // qDebug() << "myreason is: " << myreason << Qt::endl;
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString query0 = "{ \"username\" : " + slash1 + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         qx::QxRestApi cryptoapi;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query0);
//         QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         // qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
//         qx::QxRestApi cryptoapiadsupwebjson;
//         QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
//         // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
//         mypayload->processRequest(resultcryptoadsupwebjson);
//         QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
//         QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
//         QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
//         QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
//         QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
//         // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
//         //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
//         qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
//         //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;


//         if((adsupwebjsonroles>=4)) {



//             qx::QxRestApi dataapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(request.data());
//             QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
//             //  QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
//             //  qDebug() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//             mypayload->processRequest(resultdata);
//             QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
//             QJsonValue myregdata = mydearpayloaddata.value("data");
//             QJsonArray myqueryarr = myregdata.toArray();
//             QJsonDocument responsedoc(myqueryarr);
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = responsedoc.toJson(QJsonDocument::Compact);
//             errentry << " No error detected for user "  << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             sccentry << " super power operations  by superpower " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString()<< " was successful." ;




//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " ." ;
//             errentry << " super power operations  by superpower " << myrequest.value("username").toString().toStdString() <<" at "
//                      << myrequest.value("created_at").toString().toStdString() << " was not successful since the user is not admin." ;


//         }



//     });


//     // End core super power operations

//     // Start get all ids for a user

//     http.dispatch("GET", "/deliva/fetchids/<user>", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::string starter1 = R"({
//                              "action": "fetch_all",
//                              "data":
//                              {
//                                  "created_at": "2022-07-28T15:19:09.733Z",
//                                  "token": "504601637",
//                                  "username": "ericm"
//                              },
//                             "entity": "categories",
//                              "query":
//                              {
//                                  "sql": "{\"username\":\"ericm\"}"
//                              }

//                          })";

//         std::string starter2 = R"({
//                              "action": "fetch_by_query",
//                              "data":
//                              {
//                                  "created_at": "2022-07-28T15:19:09.733Z",
//                                  "token": "504601637",
//                                  "username": "auser"
//                              },
//                             "entity": "ids",
//                              "query":
//                              {
//                                  "sql": "{\"username\":\"auser\"}"
//                              },
//                            "relations": [
//                                    "*"
//                                 ]

//                          })";

//         //  QString queryparam =  request.params().value("reason");
//         QString urlparam =  request.dispatchParams().value("user").toString();
//         //  qDebug() << "query param for fetchids is: " << queryparam << Qt::endl;
//         //  qDebug() << "url param for fetchids is: " << urlparam << Qt::endl;
//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchids.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchids.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString strPayload = QString::fromStdString(starter2);
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QString myentity =  mydearpayload.value("entity").toString();
//         QString myaction =  mydearpayload.value("action").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myreason =  request.params().value("reason");
//         //  qDebug() << "myreason is: " << myreason << Qt::endl;
//         QString user1 = myreason;
//         QString user2 = urlparam;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString slash2 = + "\"" + user2 + "\"";
//         QString query0 = "{ \"username\" : " + slash1 + " }";
//         QString query1 = "{ \"username\" : " + slash2 + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         qx::QxRestApi cryptoapi;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query0);
//         QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
//         qx::QxRestApi cryptoapiadsupwebjson;
//         QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
//         // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
//         mypayload->processRequest(resultcryptoadsupwebjson);
//         QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
//         QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
//         QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
//         QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
//         QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
//         // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
//         //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
//         qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
//         //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





//         if((adsupwebjsonroles>=2)) {



//             qx::QxRestApi dataapi;
//             QJsonDocument usagedoc = QJsonDocument::fromJson(strPayload.toUtf8());
//             modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "query.sql", query1);
//             //  QString adsupwebfinaljson = usagedoc.toJson(QJsonDocument::JsonFormat::Indented);
//             // qDebug() << "adsupwebfinaljson after modification is: " << qPrintable(adsupwebfinaljson) << Qt::endl;


//             // if((myentity=="ids" || myentity=="cryptocenter"  ) && (myaction=="call_entity_function" || myaction =="call_custom_query" || myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" || myaction =="insert") ) {
//             if((myentity=="ids" || myentity=="cryptocenter" ) && ( myaction =="delete_all" || myaction =="delete_by_query" || myaction =="delete_by_id" || myaction=="call_entity_function" || myaction =="call_custom_query"  || myaction =="save" || myaction =="insert" || myaction =="count" || myaction =="validate"|| myaction =="exist" || myaction =="save" || myaction =="update" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }

//             if((myentity=="mysecrets" ) ) {

//                 modifyMyJsonToolsuser->modifyJsonValue(usagedoc, "entity", QJsonValue("nullandvoid"));

//             }


//             QString resultdata = dataapi.processRequest(usagedoc.toJson(QJsonDocument::Indented));
//             //   QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
//             //  qDebug() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//             mypayload->processRequest(resultdata);
//             QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
//             QJsonValue myregdata = mydearpayloaddata.value("data");
//             QJsonArray myqueryarr = myregdata.toArray();
//             QJsonDocument responsedoc(myqueryarr);
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = responsedoc.toJson(QJsonDocument::Compact);
//             errentry << " No error detected for user "  << urlparam.toStdString() <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             sccentry << " fetch ids for user " << urlparam.toStdString()  <<" at "
//                      <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;




//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for user " << urlparam.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             errentry << " fetch ids for user " << urlparam.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not admin." ;


//         }



//     });


//     // End get all ids for a user


//     // Start get all data from ids entity as superpower

//     http.dispatch("GET", "/deliva/fetchidssuperpower/<param>", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {


//         std::string starter1 = R"({
//                              "action": "fetch_by_query",
//                              "data":
//                              {
//                                  "created_at": "2022-07-28T15:19:09.733Z",
//                                  "token": "504601637",
//                                  "username": "auser"
//                              },
//                             "entity": "ids",
//                              "query":
//                              {
//                                  "sql": "{\"username\":\"auser\"}"
//                              },
//                            "relations": [
//                                    "*"
//                                 ]

//                          })";





//         QStringList m_destlist;
//         m_destlist << "*";
//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         QJsonArray m_array = QJsonArray::fromStringList(m_destlist);
//         QString urlparam =  request.dispatchParams().value("param").toString();
//         Wt::WLogger errlogger;
//         errlogger.addField("superpowererrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchidssuperpower.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchidssuperpower.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString strPayload = QString::fromStdString(starter1);
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myreason =  request.params().value("reason");
//         //  qDebug() << "myreason is: " << myreason << Qt::endl;
//         QString user1 = myreason;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString query0 = "{ \"username\" : " + slash1 + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         qx::QxRestApi cryptoapi;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query0);
//         QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
//         qx::QxRestApi cryptoapiadsupwebjson;
//         QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
//         // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
//         mypayload->processRequest(resultcryptoadsupwebjson);
//         QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
//         QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
//         QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
//         QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
//         QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
//         // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
//         //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
//         qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
//         //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





//         if((adsupwebjsonroles>=4) && (urlparam=="all")) {



//             qx::QxRestApi dataapi;
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_all"));
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query");
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations", m_array);
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("ids"));
//             QString adsupwebfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//             qDebug() << "adsupwebfinaljson superpower ids after modification is: " << qPrintable(adsupwebfinaljson) << Qt::endl;




//             QString resultdata = dataapi.processRequest(userdoc.toJson(QJsonDocument::Indented));
//             //   QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
//             //  qDebug() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//             mypayload->processRequest(resultdata);
//             QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
//             QJsonValue myregdata = mydearpayloaddata.value("data");
//             QJsonArray myqueryarr = myregdata.toArray();
//             QJsonDocument responsedoc(myqueryarr);
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = responsedoc.toJson(QJsonDocument::Compact);
//             errentry << " No error detected for fetch all users from entity ids by superpower  "  << myreason.toStdString() <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             sccentry << " fetch all users from entity ids by superpower  " << myreason.toStdString()  <<" at "
//                      <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;



//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for fetch all users from entity ids by superpower " << myreason.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             errentry << " fetch all users from entity ids by superpower "  << myreason.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not a superpower." ;


//         }



//     });


//     // End get all data from ids entity as superpower


//     // Start get all data from cryptocenter entity as superpower

//     http.dispatch("GET", "/deliva/fetchcryptosuperpower/<param>", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {



//         std::string starter1 = R"({
//                              "action": "fetch_by_query",
//                              "data":
//                              {
//                                  "created_at": "2022-07-28T15:19:09.733Z",
//                                  "token": "504601637",
//                                  "username": "auser"
//                              },
//                             "entity": "ids",
//                              "query":
//                              {
//                                  "sql": "{\"username\":\"auser\"}"
//                              },
//                            "relations": [
//                                    "*"
//                                 ]

//                          })";




//         QString urlparam =  request.dispatchParams().value("param").toString();
//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("superpowererrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errfetchcryptosuperpower.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccfetchcryptosuperpower.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QString strPayload = QString::fromStdString(starter1);
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myreason =  request.params().value("reason");
//         //  qDebug() << "myreason is: " << myreason << Qt::endl;
//         QString user1 = myreason;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString query0 = "{ \"username\" : " + slash1 + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "data");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         qx::QxRestApi cryptoapi;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query0);
//         QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
//         qx::QxRestApi cryptoapiadsupwebjson;
//         QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
//         // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
//         mypayload->processRequest(resultcryptoadsupwebjson);
//         QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
//         QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
//         QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
//         QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
//         QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
//         // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
//         //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
//         qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
//         //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;





//         if((adsupwebjsonroles>=4) && (urlparam=="all")) {



//             qx::QxRestApi dataapi;
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query");
//             modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_all"));
//             QString adsupwebfinaljson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//             qDebug() << "adsupwebfinaljson superpower cryptocenter after modification is: " << qPrintable(adsupwebfinaljson) << Qt::endl;
//             QString resultdata = dataapi.processRequest(userdoc.toJson(QJsonDocument::Indented));
//             //   QJsonDocument mydoc = QJsonDocument::fromJson(resultdata.toUtf8());
//             //  qDebug() << "Json Doc for admin operations resultdata is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//             mypayload->processRequest(resultdata);
//             QJsonObject mydearpayloaddata = mypayload->m_requestJson.toObject();
//             QJsonValue myregdata = mydearpayloaddata.value("data");
//             QJsonArray myqueryarr = myregdata.toArray();
//             QJsonDocument responsedoc(myqueryarr);
//             response.headers().insert("Content-Type", "application/json; charset=utf-8");
//             response.data() = responsedoc.toJson(QJsonDocument::Compact);
//             errentry << " No error detected for fetch all users from entity cryptocenter by superpower  "  << myreason.toStdString() <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             sccentry << " fetch all users from entity cryptocenter by superpower  " << myreason.toStdString()  <<" at "
//                      <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;




//         }

//         else {


//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for fetch all users from entity cryptocenter by superpower " << myreason.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             errentry << " fetch all users from entity cryptocenter by superpower "  << myreason.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not a superpower." ;


//         }



//     });


//     // End get all data from cryptocenter entity as superpower


//     // Start get token register or login


//     http.dispatch("POST", "/deliva/universal/gettoken", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errgettoken.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccgettoken.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QByteArray utfString = request.data();
//         QString strPayload = QString::fromUtf8(utfString);
//         QString queryparam =  request.params().value("reason");
//         //  QString mytoken = queryparam;
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         //  QString myfield =  mydearpayload.value("field").toString();
//         // QString myentity =  mydearpayload.value("entity").toString();
//         QString mysecret =  mydearpayload.value("secret").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myreason =  request.params().value("reason");
//         QString myusername =  myreason ;
//         //  qDebug() << "myreason is: " << myreason << Qt::endl;
//         qDebug() << "mysecret is: " << mysecret << Qt::endl;
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString query0 = "{ \"username\" : " + slash1 + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         qx::QxRestApi cryptoapi;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query0);
//         QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
//         qx::QxRestApi cryptoapiadsupwebjson;
//         QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
//         // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
//         mypayload->processRequest(resultcryptoadsupwebjson);
//         QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
//         QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
//         QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
//         QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
//         QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
//         // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
//         //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
//         qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
//         //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;

//         if((adsupwebjsonroles>=2)) {


//             eric::JsonUtils m;
//             QVariantMap usagemap = m.toVariantMap(strPayload);
//             QString desttoken = modifyMyJsonToolsuser->jwttoken(usagemap,mysecret);
//             //   qDebug() << "desttoken is: " << desttoken << Qt::endl;
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 200;
//             response.data() = desttoken.toUtf8();

//             errentry << " No error detected for admin "  << queryparam.toStdString() <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             sccentry << " get token by admin " << queryparam.toStdString()  <<" at "
//                      <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;



//         }

//         else {

//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for admin " << queryparam.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             errentry << " get token by admin " << queryparam.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not admin." ;



//         }




//     });


//     // End get token register or login


//     // Start get reason from token register


//     http.dispatch("POST", "/deliva/universal/getreasontoken", [](qx::QxHttpRequest & request, qx::QxHttpResponse & response) {

//         std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
//         Wt::WLogger errlogger;
//         errlogger.addField("systemerrors", false);
//         errlogger.setFile(mypayload->LOGS_ROOTIDS + "/errgetreasontoken.log");
//         Wt::WLogEntry errentry = errlogger.entry("error");
//         Wt::WLogger scclogger;
//         scclogger.addField("accessgranted", true);
//         scclogger.setFile(mypayload->LOGS_ROOTIDS + "/sccgetreasontoken.log");
//         Wt::WLogEntry sccentry = scclogger.entry("info");
//         QByteArray utfString = request.data();
//         //  QJsonDocument mydoc = QJsonDocument::fromJson(utfString);
//         //  qDebug() << "Json Doc in getreasontoken is: " << qPrintable(mydoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
//         QString strPayload = QString::fromUtf8(utfString);
//         QString queryparam =  request.params().value("reason");
//         //  QString mytoken = queryparam;
//         mypayload->processRequest(strPayload);
//         QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//         //  QString myfield =  mydearpayload.value("field").toString();
//         // QString myentity =  mydearpayload.value("entity").toString();
//         QString mysecret =  mydearpayload.value("secret").toString();
//         QJsonValue myvalue = mydearpayload.value("data");
//         QJsonObject myrequest = myvalue.toObject();
//         QString myreason =  request.params().value("reason");
//         QString tokenuser = myrequest.value("username").toString();
//         QString tokenjwt = myrequest.value("jwt").toString();
//         QString myusername =  myreason;
//         //  qDebug() << "myreason is: " << myreason << Qt::endl;
//         QString user1 = myusername;
//         QString slash1 = + "\"" + user1 + "\"";
//         QString query0 = "{ \"username\" : " + slash1 + " }";
//         QJsonDocument userdoc = QJsonDocument::fromJson(request.data());
//         std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("cryptocenter"));
//         qx::QxRestApi cryptoapi;
//         modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query0);
//         QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//         //  qDebug() << "adsupwebjson after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
//         qx::QxRestApi cryptoapiadsupwebjson;
//         QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
//         // qDebug() << "result after query in cryptoadsupwebjson is: " << resultcryptoadsupwebjson << Qt::endl;
//         mypayload->processRequest(resultcryptoadsupwebjson);
//         QJsonObject mydearpayloadresultsadsupwebjson = mypayload->m_requestJson.toObject();
//         QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
//         QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
//         QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
//         QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
//         // QString queryresultsadsupwebjson =  myadsupwebjsonfromquery.value("username").toString();
//         //  long long querytokenadsupwebjson =  myadsupwebjsonfromquery.value("token").toInt();
//         qint64 adsupwebjsonroles = myadsupwebjsonfromquery.value("roles").toInt();
//         //  qDebug() << "adsupwebjsonroles is: " << adsupwebjsonroles << Qt::endl;

//         if((adsupwebjsonroles>=2)) {



//             QString destreason = modifyMyJsonToolsuser->extracttoken(tokenjwt,tokenuser,mysecret);
//             //   qDebug() << "destreason is: " << destreason << Qt::endl;
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 200;
//             response.data() = destreason.toUtf8();

//             errentry << " No error detected for admin "  << queryparam.toStdString() <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             sccentry << " get reason from token by admin " << queryparam.toStdString()  <<" at "
//                      <<  modifyMyJsonToolsuser->duedate().toString().toStdString()<< " was successful." ;



//         }

//         else {

//             const std::string ErrorTemplate = "denied";
//             QString result = (QString::fromStdString(ErrorTemplate));
//             response.headers().insert("Content-Type", "text/plain; charset=iso-8859-1");
//             response.status() = 403;
//             response.data() = result.toUtf8();
//             sccentry << " Error detected for admin " << queryparam.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " ." ;
//             errentry << " get reason from token by admin " << queryparam.toStdString()  <<" at "
//                      << modifyMyJsonToolsuser->duedate().toString().toStdString() << " was not successful since the user is not admin." ;



//         }




//     });
// }

// // ------------------- start server -------------------
// bool QxHttpServerController::startServer()
// {
//     if (running_) {
//         qInfo() << "[QxHttpServerController] startServer: already running";
//         return false;
//     }

// #if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
//     QString appPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
// #else
//     QString appPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
// #endif
//     if (appPath.isEmpty()) appPath = QDir::currentPath();
//     QDir().mkpath(appPath);

//     if (!configureSqlite()) {
//         Q_EMIT errorOccurred("SQLite configuration failed");
//         return false;
//     }

//     if (!configureSsl(appPath)) {
//         qWarning() << "[QxHttpServerController] configureSsl() failed - continuing without SSL";
//     }

//     url_ = QStringLiteral("http://0.0.0.0:%1/").arg(port_);
//     Q_EMIT urlChanged();

//     running_ = true;

//     thread_ = std::thread([this, appPath]() {
//         qInfo() << "[QxHttpServerController] Server thread starting";

//         http_.reset(new qx::QxHttpServer());

//         auto cfg = qx::service::QxConnect::getSingleton();
//         if (cfg) {
//             cfg->setIp("0.0.0.0");
//             cfg->setPort(port_);
//             cfg->setThreadCount(50);
//             cfg->setKeepAlive(5000);
//             cfg->setCompressData(true);
//         } else {
//             qWarning() << "[QxHttpServerController] QxConnect singleton not available";
//         }

//         // Register QxOrm classes (must be visible/linked)
//         qx::QxClassX::registerAllClasses(true);
//         qInfo() << "[QxHttpServerController] qx::QxClassX::registerAllClasses(true) called";

//         // Configure QxSqlDatabase singleton to use sqlitePath_
//         qx::QxSqlDatabase *pDatabase = qx::QxSqlDatabase::getSingleton();
//         if (pDatabase) {
//             pDatabase->setDriverName("QSQLITE");
//             pDatabase->setDatabaseName(sqlitePath_);
//             pDatabase->setHostName("");
//             pDatabase->setUserName("");
//             pDatabase->setPassword("");
//             qInfo() << "[QxHttpServerController] qx::QxSqlDatabase configured to" << sqlitePath_;
//         } else {
//             qWarning() << "[QxHttpServerController] qx::QxSqlDatabase::getSingleton() returned null";
//         }

//         // --- Create tables using qx::dao::create_table<T>() for your domain classes ---
//         // List you requested:
//         // "categories", "cryptocenter", "delivery", "ids", "items", "location", "logistics", "metrics", "mysecrets" and "orders"
//         qInfo() << "[QxHttpServerController] Creating tables via qx::dao::create_table<...>()";
//         QSqlError daoErr;

//         // Note: the template parameter must be the class name (as declared in your orm/*.h)
//         daoErr = qx::dao::create_table<categories>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<categories>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for categories";
//         daoErr = qx::dao::create_table<cryptocenter>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<cryptocenter>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for cryptocenter";
//         daoErr = qx::dao::create_table<delivery>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<delivery>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for delivery";
//         daoErr = qx::dao::create_table<ids>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<ids>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for ids";
//         daoErr = qx::dao::create_table<items>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<items>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for items";
//         daoErr = qx::dao::create_table<locations>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<location>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for location";
//         daoErr = qx::dao::create_table<logistics>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<logistics>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for logistics";
//         daoErr = qx::dao::create_table<metrics>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<metrics>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for metrics";
//         daoErr = qx::dao::create_table<mysecrets>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<mysecrets>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for mysecrets";
//         daoErr = qx::dao::create_table<orders>(); if (daoErr.isValid()) qWarning() << "[QxOrm] create_table<orders>:" << daoErr.text(); else qInfo() << "[QxOrm] created/verified table for orders";

//         // Setup routes (paste your dispatch blocks into setupRoutes)
//         setupRoutes(*http_);

//         // per-thread DB handle in scope
//         QString connName = QStringLiteral("qx_http_thread_%1").arg((quintptr)QThread::currentThreadId());
//         if (QSqlDatabase::contains(connName)) QSqlDatabase::removeDatabase(connName);

//         {
//             QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
//             db.setDatabaseName(sqlitePath_);
//             if (!db.open()) {
//                 qWarning() << "[QxHttpServerController] Thread DB open failed:" << db.lastError().text();
//             } else {
//                 QSqlQuery q(db);
//                 q.exec("PRAGMA journal_mode = WAL;");
//             }

//             qInfo() << "[QxHttpServerController] Starting http server (this call blocks until stop)";
//             http_->startServer();
//             qInfo() << "[QxHttpServerController] http_->startServer() returned; server stopped";
//         }

//         if (QSqlDatabase::contains(connName)) {
//             QSqlDatabase::removeDatabase(connName);
//             qInfo() << "[QxHttpServerController] Removed thread DB connection:" << connName;
//         }

//         QMetaObject::invokeMethod(this, [this]() {
//             running_ = false;
//             Q_EMIT stopped();
//             Q_EMIT runningChanged();
//         }, Qt::QueuedConnection);
//     });

//     Q_EMIT started();
//     Q_EMIT runningChanged();
//     return true;
// }

// // ------------------- stop server -------------------
// void QxHttpServerController::stopServer()
// {
//     if (!running_) {
//         qInfo() << "[QxHttpServerController] stopServer: not running";
//         return;
//     }

//     qInfo() << "[QxHttpServerController] stopServer: requesting server stop";
//     try {
//         if (http_) http_->stopServer();
//     } catch (const std::exception &ex) {
//         qWarning() << "[QxHttpServerController] stop exception:" << ex.what();
//         Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//     } catch (...) {
//         qWarning() << "[QxHttpServerController] stop unknown exception";
//         Q_EMIT errorOccurred("Unknown exception while stopping server");
//     }

//     if (thread_.joinable()) thread_.join();
//     http_.reset();
//     running_ = false;
//     Q_EMIT stopped();
//     Q_EMIT runningChanged();
// }


// QVariantMap mysecretstable(qx::QxHttpRequest & request) {



//     std::string thesecretjson = R"({
//                              "action": "fetch_by_query",
//                              "data":
//                              {

//                                  "username": "Deliva"
//                              },
//                             "entity": "mysecrets",
//                              "query":
//                              {
//                                  "sql": "{\"username\":\"Deliva\"}"
//                              },
//                            "relations": [
//                                    "*"
//                                 ]

//                          })";



//     QString strPayloadsecrets = QString::fromStdString(thesecretjson);
//     std::shared_ptr<MyJsonTools> mypayloadsecrets {new MyJsonTools};
//     mypayloadsecrets->processRequest(strPayloadsecrets);
//     QJsonObject mydearpayloadsecrets = mypayloadsecrets->m_requestJson.toObject();
//     //   QString myentitysecrets =  mydearpayloadsecrets.value("entity").toString();
//     //  QString myactionsecrets =  mydearpayloadsecrets.value("action").toString();
//     QJsonValue myvaluesecrets = mydearpayloadsecrets .value("data");
//     QJsonObject myrequestsecrets = myvaluesecrets.toObject();
//     QString myusernamesecrets =  myrequestsecrets.value("username").toString();
//     QString user1 = myusernamesecrets;
//     QString slash1 = + "\"" + user1 + "\"";
//     QString query0 = "{ \"username\" : " + slash1 + " }";
//     QJsonDocument userdoc = QJsonDocument::fromJson(strPayloadsecrets.toUtf8());
//     std::shared_ptr<MyJsonTools>  modifyMyJsonToolsuser {new MyJsonTools};
//     modifyMyJsonToolsuser->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_query"));
//     modifyMyJsonToolsuser->modifyJsonValue(userdoc, "relations");
//     modifyMyJsonToolsuser->modifyJsonValue(userdoc, "entity", QJsonValue("mysecrets"));
//     qx::QxRestApi cryptoapi;
//     modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", query0);
//     QString adsupwebjson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//     //  qDebug() << "adsupwebjson mysecrets after modification is: " << qPrintable(adsupwebjson) << Qt::endl;
//     qx::QxRestApi cryptoapiadsupwebjson;
//     QString resultcryptoadsupwebjson = cryptoapi.processRequest(adsupwebjson);
//     //  qDebug() << "result after query in resultcryptoadsupwebjson secrets is: " << resultcryptoadsupwebjson << Qt::endl;
//     mypayloadsecrets->processRequest(resultcryptoadsupwebjson);
//     QJsonObject mydearpayloadresultsadsupwebjson = mypayloadsecrets->m_requestJson.toObject();
//     QJsonValue myadsupwebjsonvalue = mydearpayloadresultsadsupwebjson.value("data");
//     QJsonArray myadsupwebjsonfromqueryarr = myadsupwebjsonvalue.toArray();
//     QJsonValue dataadsupwebjsonFirstarr = myadsupwebjsonfromqueryarr.first();
//     QJsonObject myadsupwebjsonfromquery = dataadsupwebjsonFirstarr.toObject();
//     QString queryresultssecretid =  myadsupwebjsonfromquery.value("mysecrets_id").toString();
//     QString queryresultssecretusername =  myadsupwebjsonfromquery.value("username").toString();
//     QString queryresultssecretregister =  myadsupwebjsonfromquery.value("secretregister").toString();
//     QString queryresultssecretlogin =  myadsupwebjsonfromquery.value("secretlogin").toString();
//     QString queryresultssecretdate =  myadsupwebjsonfromquery.value("created_at").toString();
//     QVariantMap mydearsecrets;
//     mydearsecrets.insert("mysecrets_id", queryresultssecretid);
//     mydearsecrets.insert("secretregister", queryresultssecretregister);
//     mydearsecrets.insert("secretlogin", queryresultssecretlogin);
//     mydearsecrets.insert("username", queryresultssecretusername);
//     mydearsecrets.insert("created_at", queryresultssecretdate);

//     return mydearsecrets;


// }


/*

INSERT INTO mysecrets (
    mysecrets_id,
    username,
    secretregister,
    secretlogin
) VALUES (
    '123456789',
    'Deliva',
    'registerericdigitalbloom',
    'loginericdigitalbloom'
);


*/
