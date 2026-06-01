#include "StreamServer.hpp"

#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QHostAddress>

#include <signal.h>
#include <chrono>
#include <thread>
#include <map>
#include <mutex>

#include "Util/File.h"
#include "Util/logger.h"
#include "Util/SSLBox.h"
#include "Util/onceToken.h"
#include "Util/MD5.h"
#include "Network/TcpServer.h"
#include "Rtmp/RtmpSession.h"
#include "Rtmp/FlvMuxer.h"
#include "Player/PlayerProxy.h"
#include "Http/WebSocketSession.h"
#include "Common/config.h"

// bring the same namespaces as in your codebase
using namespace std;
using namespace toolkit;
using namespace mediakit;

//
// --- mediakit namespace default-port initialization (exact as provided) ---
//
namespace mediakit {
////////////HTTP configuration///////////
namespace Http {
#define HTTP_FIELD "http."
#define HTTP_PORT 8184
const string kPort = HTTP_FIELD"port";
#define HTTPS_PORT 4184
const string kSSLPort = HTTP_FIELD"sslport";
onceToken token1([](){
    mINI::Instance()[kPort] = HTTP_PORT;
    mINI::Instance()[kSSLPort] = HTTPS_PORT;
},nullptr);
} // namespace Http

////////////SHELL configuration///////////
namespace Shell {
#define SHELL_FIELD "shell."
#define SHELL_PORT 9000
const string kPort = SHELL_FIELD"port";
onceToken token1([](){
    mINI::Instance()[kPort] = SHELL_PORT;
},nullptr);
} // namespace Shell

////////////RTSP server configuration///////////
namespace Rtsp {
#define RTSP_FIELD "rtsp."
#define RTSP_PORT 554
#define RTSPS_PORT 322
const string kPort = RTSP_FIELD"port";
const string kSSLPort = RTSP_FIELD"sslport";
onceToken token1([](){
    mINI::Instance()[kPort] = RTSP_PORT;
    mINI::Instance()[kSSLPort] = RTSPS_PORT;
},nullptr);
} // namespace Rtsp

////////////RTMP server configuration///////////
namespace Rtmp {
#define RTMP_FIELD "rtmp."
#define RTMP_PORT 1938
#define RTMPS_PORT 19380
const string kPort = RTMP_FIELD"port";
const string kSSLPort = RTMP_FIELD"sslport";
onceToken token1([](){
    mINI::Instance()[kPort] = RTMP_PORT;
    mINI::Instance()[kSSLPort] = RTMPS_PORT;
},nullptr);
} // namespace Rtmp

} // namespace mediakit

#define REALM "STREAM SERVER"
static map<string,FlvRecorder::Ptr> s_mapFlvRecorder;
static mutex s_mtxFlvRecorder;

#if !defined(SIGHUP)
#define SIGHUP 1
#endif

// Forward declaration of your existing functions (config.cpp)
namespace mediakit {
bool loadIniConfig(const char *ini_path);
void initEventListener();
}

// --- initEventListener implementation (copied from your original) ---
void mediakit::initEventListener() {
    try {
        static onceToken s_token([]() {
            // Listen to the kBroadcastOnGetRtspRealm event to determine whether the rtsp link needs authentication
            NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastOnGetRtspRealm,
                                                 [](BroadcastOnGetRtspRealmArgs) {
                                                     DebugL << "Whether RTSP requires authentication event:" << args._schema << "" << args._vhost << ""
                                                            << args._app << " " << args._streamid << " "
                                                            << args._param_strs;
                                                     if (string("1") == args._streamid) {
                                                         // live/1 requires authentication -> set realm
                                                         invoker(REALM);
                                                     } else {
                                                         // no authentication required
                                                         invoker("");
                                                     }
                                                 });

            // Listen to the kBroadcastOnRtspAuth event to return the correct rtsp authentication user password
            NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastOnRtspAuth, [](BroadcastOnRtspAuthArgs) {
                DebugL << "RTSP playback authentication:" << args._schema << "" << args._vhost << "" << args._app << "" << args._streamid
                       << "" << args._param_strs;
                DebugL << "RTSP user:" << user_name << (must_no_encrypt? "Base64": "MD5") << "Log in by way";
                string user = user_name;
                // Suppose we read the database asynchronously
                if (user == "test0") {
                    // Assuming that the database saves plaintext
                    invoker(false, "pwd0");
                    return;
                }

                if (user == "test1") {
                    // Assuming that the database saves ciphertext
                    auto encrypted_pwd = MD5(user + ":" + REALM + ":" + "pwd1").hexdigest();
                    invoker(true, encrypted_pwd);
                    return;
                }
                if (user == "test2" && must_no_encrypt) {
                    // Base64 login mismatch example
                    invoker(true, "pwd2");
                    return;
                }

                // Other user passwords are the same as the user name
                invoker(false, user);
            });

            // Listen to rtsp/rtmp push events, and return the result to inform whether there is push permission
            NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastMediaPublish, [](BroadcastMediaPublishArgs) {
                DebugL << "Push Stream Authentication:" << args._schema << "" << args._vhost << "" << args._app << "" << args._streamid << ""
                       << args._param_strs;

                auto parser = make_unique<Parser>();
                std::string data = parser->parseArgs(args._param_strs)["business"];

                // Example: allow push (empty error + default ProtocolOption)
                invoker("", ProtocolOption());
                // To reject: invoker("auth failed message", ProtocolOption());
            });

            // Listen to playback events to decide playback permission
            NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastMediaPlayed, [](BroadcastMediaPlayedArgs) {
                DebugL << "Play authentication:" << args._schema << "" << args._vhost << "" << args._app << "" << args._streamid << ""
                       << args._param_strs;
                invoker(""); // allow
                // invoker("denied"); // deny
            });

            // shell login event
            NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastShellLogin, [](BroadcastShellLoginArgs) {
                DebugL << "shell login:" << user_name << " " << passwd;
                invoker("");//Authentication is successful
                //invoker("this is auth failed message");//Authentication failed
            });

            // Listening to play failure (no specific stream found) event
            NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastNotFoundStream,
                                                 [](BroadcastNotFoundStreamArgs) {
                                                     DebugL << "Stream event not found:" << args._schema << "" << args._vhost << ""
                                                            << args._app << "" << args._streamid << ""
                                                            << args._param_strs;
                                                 });

            // Monitor the data consumption event at the end of playback or push
            NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastFlowReport, [](BroadcastFlowReportArgs) {
                DebugL << "Player (streamer) disconnection event: "<< args._schema <<" "<< args._vhost <<" "<< args._app <<" "
                       << args._streamid << " " << args._param_strs
                       << "\r\nUsing traffic:" << totalBytes << "bytes, connection duration:" << totalDuration << "seconds";
            });

        }, nullptr);

    } catch (std::exception& e) {
        std::cerr << "database exception is : " << e.what() << "\n";
    }
}

//
// --- Controller implementation (Qt thread wrapper and ini loading) ---
//

StreamServer::StreamServer(QObject *parent)
    : QObject(parent)
{
}

StreamServer::~StreamServer()
{
    stopServer();
}

QString StreamServer::downloadLocation()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    QString loc = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#else
    QString loc = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif
    if (loc.isEmpty()) loc = QDir::currentPath();
    if (!loc.endsWith(QDir::separator())) loc += QDir::separator();
    return loc;
}

QString StreamServer::resolveIniPath()
{
    const QString iniName = QString::fromUtf8("zlmediakit.ini");
    QString downloads = downloadLocation();
    QString candidate = downloads + iniName;

    // If the user placed an ini in Downloads, prefer it
    if (QFile::exists(candidate)) {
        return candidate;
    }

    // fallback: exeDir() + "<appname>.ini" (same as your dump default behavior)
    QString exeDirUtf8 = QString::fromLocal8Bit(exeDir().c_str());
    if (!exeDirUtf8.endsWith(QDir::separator())) exeDirUtf8 += QDir::separator();
    QString exeIni = exeDirUtf8 + QFileInfo(QCoreApplication::applicationFilePath()).baseName() + QStringLiteral(".ini");

    // return exeIni if it exists, otherwise return candidate (even if missing) so loadIniConfig will create defaults
    if (QFile::exists(exeIni)) return exeIni;
    return candidate;
}

bool StreamServer::startServer()
{
    if (running_.load()) {
        qInfo() << "[StreamServer] already running";
        return false;
    }

    stopRequested_.store(false);

    thr_ = std::make_unique<std::thread>(&StreamServer::threadMain, this);

    // small wait to allow thread to initialize
    using clock = std::chrono::steady_clock;
    auto start = clock::now();
    while (!running_.load() && std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start).count() < 2000) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return running_.load();
}

void StreamServer::stopServer()
{
    if (!running_.load()) {
        qInfo() << "[StreamServer] not running";
        return;
    }

    stopRequested_.store(true);

    // If ZLMediaKit provides global shutdown APIs, call them here (best-effort)
    try {
        // e.g. EventPoller::Instance().shutdown();
    } catch (...) {}

    if (thr_ && thr_->joinable()) thr_->join();
    thr_.reset();
}

// ----------------- Address discovery helpers ------------------

// Return a list of candidate IP addresses. By default returns IPv4 only.
// Set includeV6=true to also include IPv6 addresses (link-local and global).
QStringList StreamServer::getLocalIps(bool includeV6) {
    QStringList out;

    // First pass: prefer physical interfaces (same heuristic as before)
    const QStringList preferredPrefixes = { "eth", "en", "wl", "wlan", "enp", "eno" };
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
        QString name = iface.humanReadableName().toLower();

        // Only prefer physical-like interfaces in first pass
        bool preferred = false;
        for (const QString &p : preferredPrefixes) {
            if (name.startsWith(p)) { preferred = true; break; }
        }
        if (!preferred) continue;

        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            QHostAddress addr = entry.ip();
            if (addr.isNull()) continue;
            if (!includeV6 && addr.protocol() != QAbstractSocket::IPv4Protocol) continue;
            if (addr.isLoopback()) continue;
            // Skip IPv6 link-local unless explicitly requested (but if includeV6==true we keep them)
            if (addr.protocol() == QAbstractSocket::IPv6Protocol) {
                out << addr.toString(); // may include scope like "%en0"
            } else {
                out << addr.toString();
            }
        }
    }

    // Second pass: any non-loopback address on up interfaces
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            QHostAddress addr = entry.ip();
            if (addr.isNull()) continue;
            if (!includeV6 && addr.protocol() != QAbstractSocket::IPv4Protocol) continue;
            if (addr.isLoopback()) continue;
            QString s = addr.toString();
            if (!out.contains(s)) out << s;
        }
    }

    // Last fallback: any address discovered via allAddresses()
    const auto allAddrs = QNetworkInterface::allAddresses();
    for (const QHostAddress &addr : allAddrs) {
        if (addr.isNull()) continue;
        if (!includeV6 && addr.protocol() != QAbstractSocket::IPv4Protocol) continue;
        if (addr.isLoopback()) continue;
        QString s = addr.toString();
        if (!out.contains(s)) out << s;
    }

    return out;
}

// Return preferred IPv4 address (unchanged behavior)
QString StreamServer::getLocalIp() {
    QStringList list = getLocalIps(false);
    return list.isEmpty() ? QString() : list.first();
}

// Return a preferred IPv6 address (global > link-local). If none, return empty.
QString StreamServer::getLocalIpV6() {
    // Get IPv6 addresses including link-local
    QStringList addrs = getLocalIps(true);
    if (addrs.isEmpty()) return QString();

    // Prefer global-scoped IPv6 (no leading "fe80"), then link-local as fallback
    for (const QString &a : addrs) {
        if (a.contains(':') && !a.startsWith("fe80", Qt::CaseInsensitive)) {
            return a;
        }
    }
    // fallback to first IPv6 (maybe link-local)
    for (const QString &a : addrs) {
        if (a.contains(':')) return a;
    }
    return QString();
}

// New: setter for port exposed to QML/C++
void StreamServer::setPort(int p)
{
    if (p <= 0) return; // ignore invalid
    int old = port_;
    if (old == p) return;

    // Update in-memory and mINI so config persists/reflects choice
    port_ = p;
    mINI::Instance()[Rtmp::kPort] = p; // keep mINI consistent for other components

    emit portChanged();

    // If running, restart server to apply new port (safe and simple)
    if (running_.load()) {
        qInfo() << "[StreamServer] port changed while running; restarting server to apply new port:" << p;
        stopServer();
        // small delay to allow resources to be freed
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        startServer();
    }
}

// New: set explicit listen IP (not 0.0.0.0 / loopback).
// If server is running, restart to apply new IP.
void StreamServer::setListenIp(const QString &ip)
{
    if (ip == listenIp_) return;
    listenIp_ = ip;
    emit listenIpChanged();

    // Update mINI so the config remains persistent
    if (!listenIp_.isEmpty()) {
        mINI::Instance()[General::kListenIP] = listenIp_.toStdString();
    }

    if (running_.load()) {
        qInfo() << "[StreamServer] listen IP changed while running; restarting server to bind to:" << listenIp_;
        stopServer();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        startServer();
    }
}

void StreamServer::threadMain()
{
    try {
        // 1) logging
        Logger::Instance().add(std::make_shared<ConsoleChannel>());
        Logger::Instance().add(std::make_shared<FileChannel>());
        Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

        // 2) load ini file from downloads (or exe fallback)
        QString iniPath = resolveIniPath();
        qInfo() << "[StreamServer] calling loadIniConfig with:" << iniPath;
        loadIniConfig(iniPath.isEmpty() ? nullptr : iniPath.toLocal8Bit().constData());

        // 3) init event listeners (unchanged)
        initEventListener();

        // 4) SSL init (same as your main.cpp)
        QString sslPem = QString::fromLocal8Bit(exeDir().c_str()) + QStringLiteral("ssl.pem");
        if (File::is_file(sslPem.toStdString())) {
            SSL_Initor::Instance().loadCertificate(sslPem.toStdString().data());
            SSL_Initor::Instance().trustCertificate(sslPem.toStdString().data());
            SSL_Initor::Instance().ignoreInvalidCertificate(false);
            InfoL << "Loaded ssl pem from " << sslPem.toStdString();
        } else {
            InfoL << "ssl.pem not found in exeDir; skipping SSL init";
        }

        // 5) Read configured ports and decide listen IP
        // Respect explicit listenIp_ set by QML/C++; otherwise read from mINI or use discovered IP
        QString listen;
        if (!listenIp_.isEmpty()) {
            listen = listenIp_;
        } else {
            std::string listenIpCfg = mINI::Instance()[General::kListenIP];
            listen = QString::fromUtf8(listenIpCfg.data());
            if (listen.isEmpty() || listen == QStringLiteral("::") || listen == QStringLiteral("0.0.0.0")) {
                // Instead of binding to 0.0.0.0 or loopback, if the user hasn't explicitly set a listen IP
                // we can query host IP and use it (so server binds to a usable interface).
                QString ip = getLocalIp(); // prefer IPv4 by default
                if (!ip.isEmpty()) {
                    listen = ip;
                } else {
                    // fallback to 0.0.0.0 if nothing else
                    listen = QStringLiteral("0.0.0.0");
                }
            }
        }
        listenIp_ = listen;
        QMetaObject::invokeMethod(this, [this]() { emit listenIpChanged(); });

        // choose port: if port_ set by setPort() use it, else read from mINI/defaults
        uint16_t rtmpPort = 0;
        if (port_ > 0) rtmpPort = static_cast<uint16_t>(port_);
        else rtmpPort = static_cast<uint16_t>(mINI::Instance()[Rtmp::kPort]);

        uint16_t rtmpsPort = static_cast<uint16_t>(mINI::Instance()[Rtmp::kSSLPort]);
        uint16_t httpsPort = static_cast<uint16_t>(mINI::Instance()[Http::kSSLPort]);

        InfoL << "Starting servers: RTMP=" << rtmpPort << " RTMPS=" << rtmpsPort << " HTTPS=" << httpsPort
              << " listen=" << listen.toStdString();

        // 6) Create servers
        TcpServer::Ptr rtmpSrvs(new TcpServer());
        TcpServer::Ptr rtmpSrv(new TcpServer());
        TcpServer::Ptr httpsSrv(new TcpServer());

        // Start servers binding to the chosen listen IP (explicit, discovered, or fallback)
        rtmpSrv->start<RtmpSession>(rtmpPort, listen.toStdString());
        if (rtmpsPort) rtmpSrvs->start<RtmpSessionWithSSL>(rtmpsPort, listen.toStdString());
        if (httpsPort) httpsSrv->start<HttpsSession>(httpsPort, listen.toStdString());

        // 7) Listen for reload events (unchanged)
        NoticeCenter::Instance().addListener(ReloadConfigTag, Broadcast::kBroadcastReloadConfig, [&](BroadcastReloadConfigArgs){
            try {
                if (rtmpsPort != mINI::Instance()[Rtmp::kSSLPort].as<uint16_t>()) {
                    rtmpsPort = mINI::Instance()[Rtmp::kSSLPort];
                    rtmpSrvs->start<RtmpSessionWithSSL>(rtmpsPort, listen.toStdString());
                    InfoL << "Restarted RTMPS server:" << rtmpsPort;
                }
                if (httpsPort != mINI::Instance()[Http::kSSLPort].as<uint16_t>()) {
                    httpsPort = mINI::Instance()[Http::kSSLPort];
                    httpsSrv->start<HttpsSession>(httpsPort, listen.toStdString());
                    InfoL << "Restarted HTTPS server:" << httpsPort;
                }
                if (rtmpPort != mINI::Instance()[Rtmp::kPort].as<uint16_t>()) {
                    rtmpPort = mINI::Instance()[Rtmp::kPort];
                    rtmpSrv->start<RtmpSession>(rtmpPort, listen.toStdString());
                    InfoL << "Restarted RTMP server:" << rtmpPort;
                }
            } catch (std::exception &ex) {
                WarnL << "Error while reloading config: " << ex.what();
            }
        });

        // 8) Ready
        running_.store(true);
        QMetaObject::invokeMethod(this, [this]() {
            emit started();
            emit runningChanged();
        }, Qt::QueuedConnection);

        // 9) Wait for stop
        while (!stopRequested_.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        // 10) Shutdown best-effort
        InfoL << "StreamServer: shutdown requested, cleaning up.";
        try { if (httpsSrv) httpsSrv->shutdown(); } catch(...) {}
        try { if (rtmpsPort) rtmpSrvs->shutdown(); } catch(...) {}
        try { if (rtmpSrv) rtmpSrv->shutdown(); } catch(...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        running_.store(false);
        QMetaObject::invokeMethod(this, [this]() {
            emit stopped();
            emit runningChanged();
        }, Qt::QueuedConnection);

    } catch (const std::exception &ex) {
        QString err = QString::fromUtf8(ex.what());
        QMetaObject::invokeMethod(this, [this, err]() { emit errorOccurred(err); }, Qt::QueuedConnection);
        running_.store(false);
        QMetaObject::invokeMethod(this, [this]() { emit runningChanged(); }, Qt::QueuedConnection);
    } catch (...) {
        QMetaObject::invokeMethod(this, [this]() { emit errorOccurred(QStringLiteral("Unknown exception in media thread")); }, Qt::QueuedConnection);
        running_.store(false);
        QMetaObject::invokeMethod(this, [this]() { emit runningChanged(); }, Qt::QueuedConnection);
    }
}
