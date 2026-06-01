#include "servercontrollerwt.h"



#include "base/UserNameResource.h"
#include "base/MapperSoftware.h"
#include "base/ServerResource.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include <vector>
#include <string>
#include <csignal>
#include <atomic>
#include <iostream>
#include <archive.h>
#include <archive_entry.h>
#include <QResource>
#include "LibArchiveExtractor.h"
#include "MyJsonTools.hpp"


// Your project headers -- do not duplicate mapper or server resource implementation here.
// Make sure these headers exist in your repo and contain the definitions.
#include "base/ServerResource.h"    // defines ServerResource
// CopyAssets may be defined in your project. Include its header if you have one.
#include "CopyAssets.h"            // defines CopyAssets (cpDir)



// Start Server Ops



namespace {

const std::string showMenuText = "<i class='fa fa-bars' aria-hidden='true'></i> Show menu";
const std::string closeMenuText = "<i class='fa fa-bars' aria-hidden='true'></i> Close menu";

}





  MapperSoftware::MapperSoftware (const Wt::WEnvironment& myenv)
    : Wt::WApplication(myenv),openMenuButton_(nullptr),menuOpen_(false)
  {

  }


if(thecount+1==allrows) {

 thecount=0;

}



  ServerResource::ServerResource()

   {



   }

  ServerResource::~ServerResource()

   {
      beingDeleted();
   }


   std::string ServerResource::fromIstream(std::istream &stream)
    {
          std::istreambuf_iterator<char> eos;
          return std::string(std::istreambuf_iterator<char>(stream), eos);
    }


   void ServerResource::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) { // begin virtual void handleRequest

   
   }





 static const char *OrdersUrl = "/";




std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env)
{
  return std::make_unique<MapperSoftware>(env);
}


QStringList ServerController::availableIPAddresses() {
    QStringList list;
    const auto addrs = QNetworkInterface::allAddresses();
    for (const QHostAddress &addr : addrs) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && !addr.isLoopback()) {
            list << addr.toString();
        }
    }
    // If no non-loopback IPv4 found, include loopback as fallback
    if (list.isEmpty()) {
        list << QStringLiteral("127.0.0.1");
    }
    return list;
}



// start best working
// top of file: only declare environ on POSIX non-Android targets
#if !defined(WT_WIN32) && !defined(__ANDROID__)
extern char **environ; // for Wt::WServer::restart on POSIX (not Android)
#endif

namespace {
    std::atomic<int> g_lastSignal{0};
    void signalHandler(int sig) {
        g_lastSignal.store(sig);
    }
}

ServerController::ServerController(QObject *parent)
    : QObject(parent)
{ }

ServerController::~ServerController() {
    stopServer();
}


bool ServerController::startServer(int port) {
    if (running_.load()) return false;
    running_ = true;

    m_url = QStringLiteral("http://0.0.0.0:%1/").arg(port);
    Q_EMIT urlChanged();

    QString downloadsPath = "";

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    downloadsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#else
    downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif

    if (downloadsPath.isEmpty())
        downloadsPath = QDir::currentPath();

    qInfo() << "[ServerController] Using downloadsPath:" << downloadsPath;

    QDir baseDir(downloadsPath);
    if (!baseDir.exists("Orders_files")) {
        qInfo() << "[ServerController] Creating Orders_files folder in" << baseDir.absolutePath();
        if (!baseDir.mkdir("Orders_files")) {
            qWarning() << "[ServerController] Failed to create Orders_files folder in" << baseDir.absolutePath();
        }
    }
    baseDir.cd("Orders_files");

    QString appPath = baseDir.filePath("App");
    qInfo() << "[ServerController] appPath will be:" << appPath;

    if (!QDir(appPath).exists()) {
        qInfo() << "[ServerController] Extracting AppArchive from QRC...";
        QFile qrcFile(":/myserverassets/AppArchive");
        if (!qrcFile.exists()) {
            qWarning() << "[ServerController] QRC resource missing: :/myserverassets/AppArchive";
            Q_EMIT errorOccurred("QRC resource missing");
            running_ = false;
            return false;
        }

        if (!qrcFile.open(QIODevice::ReadOnly)) {
            qWarning() << "[ServerController] Failed to open QRC archive file:" << qrcFile.errorString();
            Q_EMIT errorOccurred("Failed to open archive from QRC: " + qrcFile.errorString());
            running_ = false;
            return false;
        }

        QByteArray data = qrcFile.readAll();
        qrcFile.close();

        qInfo() << "[ServerController] Read archive size (bytes):" << data.size();
        if (data.isEmpty()) {
            qWarning() << "[ServerController] Archive read empty — resource probably not packaged.";
            Q_EMIT errorOccurred("Archive empty");
            running_ = false;
            return false;
        }

        LibArchiveExtractor extractor;
        qInfo() << "[ServerController] Calling extractor.extractTarXzFromMemory(dest:" << baseDir.absolutePath() << ") ...";
        if (!extractor.extractTarXzFromMemory(data.data(), static_cast<size_t>(data.size()), baseDir.absolutePath())) {
            qWarning() << "[ServerController] Extraction failed (see extractor logs).";
            Q_EMIT errorOccurred("Extraction failed from memory");
            running_ = false;
            return false;
        }
        qInfo() << "[ServerController] Archive extracted to" << appPath;
    } else {
        qInfo() << "[ServerController] Archive already extracted to" << appPath << ", skipping extraction.";
    }

    QString approotDir = QDir(appPath).filePath("approot");
    QString docrootDir = QDir(appPath).filePath("docroot");
    QString resourcesDir = QDir(appPath).filePath("resources");

    qInfo() << "[ServerController] Checking paths:\n appPath =" << appPath
            << "\n approotDir =" << approotDir
            << "\n docrootDir =" << docrootDir
            << "\n resourcesDir =" << resourcesDir;

    qInfo() << "[ServerController] exists(appPath):" << QDir(appPath).exists()
            << " exists(approot):" << QDir(approotDir).exists()
            << " exists(docroot):" << QDir(docrootDir).exists();

    if (!QDir(approotDir).exists() || !QDir(docrootDir).exists()) {
        qWarning() << "[ServerController] Required folders not found after extraction.";
        Q_EMIT errorOccurred("Required folders missing");
        running_ = false;
        return false;
    }

    m_assetPath = appPath;
    Q_EMIT assetPathChanged();

    // Start the server thread
    serverThread_ = std::make_unique<std::thread>([this, port, approotDir, docrootDir, resourcesDir]() mutable {
        try {
            qInfo() << "[ServerThread] Building WServer argument list...";
            std::vector<std::string> argHolder;
            QStringList qargs;
            qargs << "Orders"
                  << "--http-address=0.0.0.0"
                  << "--http-port=" + QString::number(port)
                  << "--docroot=" + docrootDir
                  << "--approot=" + approotDir
                  << "--resources-dir=" + resourcesDir;

            qInfo() << "[ServerThread] qargs:";
            for (const QString &a : qargs) qInfo() << "  " << a;

            for (const QString &arg : qargs) {
                argHolder.push_back(arg.toStdString());
            }

            std::vector<char*> argv;
            argv.reserve(argHolder.size() + 1);
            for (auto &s : argHolder) {
                argv.push_back(const_cast<char*>(s.c_str()));
            }
            argv.push_back(nullptr);
            int argc = static_cast<int>(argv.size() - 1);

            qInfo() << "[ServerThread] argc =" << argc;
            for (int i = 0; i < argc; ++i) {
                qInfo() << "[ServerThread] argv[" << i << "] =" << (argv[i] ? argv[i] : "<null>");
            }

            // ----- Port probe (diagnostic) -----
            {
                QTcpServer probeAny;
                bool bindAny = probeAny.listen(QHostAddress::Any, port);
                qInfo() << "[PortProbe] bind Any (" << port << ") ->" << (bindAny ? "OK" : QString("FAIL (%1)").arg(probeAny.errorString()));
                if (bindAny) probeAny.close();
            }
            {
                QTcpServer probe127;
                bool bind127 = probe127.listen(QHostAddress::LocalHost, port);
                qInfo() << "[PortProbe] bind 127.0.0.1 (" << port << ") ->" << (bind127 ? "OK" : QString("FAIL (%1)").arg(probe127.errorString()));
                if (bind127) probe127.close();
            }

            qInfo() << "[ServerThread] Constructing WServer...";
            try {
                server_ = std::make_unique<Wt::WServer>(argc, argv.data(), WTHTTP_CONFIGURATION);
                qInfo() << "[ServerThread] WServer constructed successfully";
            } catch (const std::exception &ex) {
                qWarning() << "[ServerThread] WServer construction failed:" << ex.what();
                Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
                running_ = false;
                QMetaObject::invokeMethod(this, [this]() { Q_EMIT stopped(); }, Qt::QueuedConnection);
                return;
            }

            ServerResource myserverResource;

            qInfo() << "[ServerThread] Adding resources and entry points...";
            try {
                server_->addResource(&myserverResource, "/deliva/${data}/${business}");
                server_->addEntryPoint(Wt::EntryPointType::Application,
                                      std::bind(&createApplication, std::placeholders::_1),
                                      OrdersUrl);
                qInfo() << "[ServerThread] addResource/addEntryPoint OK";
            } catch (const std::exception &ex) {
                qWarning() << "[ServerThread] Exception while adding resource/entrypoint:" << ex.what();
                Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
            }

            qInfo() << "[ServerThread] Attempting server_->start()...";
            bool serverStarted = false;

            try {
                serverStarted = server_->start();
            } catch (const std::exception &ex) {
                qWarning() << "[ServerThread] server_->start() threw exception:" << ex.what();
                Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
            } catch (...) {
                qWarning() << "[ServerThread] server_->start() threw unknown exception";
                Q_EMIT errorOccurred("Unknown exception when starting server");
            }

            qInfo() << "[ServerThread] server_->start() returned:" << serverStarted;

            if (serverStarted) {
                QMetaObject::invokeMethod(this, [this]() { Q_EMIT started(); }, Qt::QueuedConnection);
                qInfo() << "[ServerThread] Emitted started()";

                // ---- Interface enumeration and self-connection tests ----
                qInfo() << "[ServerThread] Enumerating network interfaces and addresses:";
                for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
                    if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
                    QStringList addrs;
                    for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                        addrs << entry.ip().toString();
                    }
                    qInfo() << "[NetIf]" << iface.humanReadableName() << iface.name() << "flags=" << iface.flags() << "addrs=" << addrs;
                }

                qInfo() << "[ServerThread] Local self-connection tests:";
                for (const QHostAddress &addr : QNetworkInterface::allAddresses()) {
                    if (addr.protocol() != QAbstractSocket::IPv4Protocol) continue;
                    if (addr.isLoopback()) continue; // skip loopback here
                    QTcpSocket sock;
                    sock.connectToHost(addr, port);
                    bool ok = sock.waitForConnected(500);
                    qInfo() << "[SelfTest] connect to" << addr.toString() << ":" << port << " -> " << (ok ? "OK" : QString("FAIL (%1)").arg(sock.errorString()));
                    if (ok) sock.disconnectFromHost();
                }
                qInfo() << "[ServerThread] Self tests complete";

                QCoreApplication *qcore = QCoreApplication::instance();
                if (qcore) {
                    qInfo() << "[ServerThread] QCoreApplication::instance() type:" << qcore->metaObject()->className();
                    QObject::connect(qcore, &QCoreApplication::aboutToQuit, this, [this]() {
                        qInfo() << "[ServerThread] aboutToQuit -> stopping server";
                        try { if (server_) server_->stop(); } catch (...) {}
                    }, Qt::QueuedConnection);
                }

                // Do not register SIGHUP on Android; only register on non-Windows non-Android POSIX
#if !defined(WT_WIN32) && !defined(__ANDROID__)
                signal(SIGHUP, signalHandler);
#endif
#ifdef SIGTERM
                signal(SIGTERM, signalHandler);
#endif
#ifdef SIGABRT
                signal(SIGABRT, signalHandler);
#endif
#ifdef SIGINT
                signal(SIGINT, signalHandler);
#endif

                qInfo() << "[ServerThread] Waiting for shutdown...";
                int sig = Wt::WServer::waitForShutdown();
                qInfo() << "[ServerThread] waitForShutdown returned sig=" << sig;

#if !defined(WT_WIN32) && !defined(__ANDROID__)
                if (sig == SIGHUP) {
                    qInfo() << "[ServerThread] Received SIGHUP; attempting restart...";
                    try {
                        Wt::WServer::restart(argc, argv.data(), environ);
                    } catch (const std::exception &ex) {
                        qWarning() << "[ServerThread] restart failed:" << ex.what();
                        Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
                    } catch (...) {
                        qWarning() << "[ServerThread] restart failed with unknown exception";
                        Q_EMIT errorOccurred("WServer restart failed with unknown exception");
                    }
                }
#endif
            } else {
                qWarning() << "[ServerThread] server_->start() returned false";
                Q_EMIT errorOccurred("WServer failed to start (returned false)");
            }

            try {
                if (server_) {
                    qInfo() << "[ServerThread] Stopping server_ ...";
                    server_->stop();
                }
            } catch (const std::exception &ex) {
                qWarning() << "[ServerThread] Exception while stopping server_:" << ex.what();
            } catch (...) {
                qWarning() << "[ServerThread] Unknown exception while stopping server_";
            }

        } catch (...) {
            qWarning() << "[ServerThread] Unknown exception in server thread";
            Q_EMIT errorOccurred("Unknown exception in server thread");
        }

        running_ = false;
        QMetaObject::invokeMethod(this, [this]() { Q_EMIT stopped(); }, Qt::QueuedConnection);
    });

    return true;
}


void ServerController::stopServer() {
    if (!running_.load()) return;

    if (server_) {
        try {
            server_->stop();
        } catch (const std::exception &ex) {
            Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
        } catch (...) {
            Q_EMIT errorOccurred(QStringLiteral("Unknown exception while stopping server"));
        }
    }

    if (serverThread_ && serverThread_->joinable()) {
        serverThread_->join();
    }

    serverThread_.reset();
    server_.reset();
    running_ = false;
    Q_EMIT stopped();
}



