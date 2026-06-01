// /mnt/data/StreamServer.cpp
// Updated StreamServer (Qt6) — enforces non-loopback selection & sets Http::kRootPath before start

#include "StreamServer.hpp"

#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QMetaObject>

#include <thread>
#include <chrono>
#include <atomic>

extern "C" {
#include "zlm_wrapper_api.h"
}

#include "Common/config.h" // for Http::kRootPath constant
#include "Util/logger.h"

using namespace std;
using namespace toolkit;
using namespace mediakit;

static inline bool isLoopbackOrWildcard(const QString &ip) {
    if (ip.isEmpty()) return true;
    if (ip == QLatin1String("0.0.0.0")) return true;
    if (ip == QLatin1String("::") || ip == QLatin1String("::0")) return true;
    if (ip == QLatin1String("::1")) return true;
    if (ip.startsWith(QLatin1String("127."))) return true;
    return false;
}

StreamServer::StreamServer(QObject *parent)
    : QObject(parent)
    , zlm_(nullptr)
    , running_(false)
{
    zlm_ = zlm_create();
}

StreamServer::~StreamServer()
{
    stopServer();
    if (zlm_) {
        zlm_destroy(zlm_);
        zlm_ = nullptr;
    }
}

QString StreamServer::downloadLocation()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    QString loc = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#else
    QString loc = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif
    if (loc.isEmpty()) loc = QDir::currentPath();
    if (!loc.endsWith(QDir::separator())) loc += QDir::separator();
    return loc;
}

QString StreamServer::computeIniPath()
{
    QString dl = downloadLocation();
    return dl + QStringLiteral("streamserver.ini");
}

QString StreamServer::computeSslPath()
{
    QString dl = downloadLocation();
    return dl + QStringLiteral("ssl.pem");
}

QStringList StreamServer::getLocalIps(bool includeV6)
{
    QStringList out;
    const QStringList preferredPrefixes = { "eth", "en", "wl", "wlan", "enp", "eno" };

    // first pass: preferred interface names
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
        QString name = iface.humanReadableName().toLower();
        bool preferred = false;
        for (const QString &p : preferredPrefixes) {
            if (name.startsWith(p)) { preferred = true; break; }
        }
        if (!preferred) continue;
        for (const QNetworkAddressEntry &e : iface.addressEntries()) {
            QHostAddress a = e.ip();
            if (a.isNull()) continue;
            if (!includeV6 && a.protocol() != QAbstractSocket::IPv4Protocol) continue;
            if (a.isLoopback()) continue;
            out << a.toString();
        }
    }

    // second pass: any non-loopback
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
        for (const QNetworkAddressEntry &e : iface.addressEntries()) {
            QHostAddress a = e.ip();
            if (a.isNull()) continue;
            if (!includeV6 && a.protocol() != QAbstractSocket::IPv4Protocol) continue;
            if (a.isLoopback()) continue;
            if (!out.contains(a.toString())) out << a.toString();
        }
    }

    // fallback: all addresses
    for (const QHostAddress &addr : QNetworkInterface::allAddresses()) {
        if (addr.isNull()) continue;
        if (!includeV6 && addr.protocol() != QAbstractSocket::IPv4Protocol) continue;
        if (addr.isLoopback()) continue;
        if (!out.contains(addr.toString())) out << addr.toString();
    }

    return out;
}

QString StreamServer::getLocalIp()
{
    auto l = getLocalIps(false);
    return l.isEmpty() ? QString() : l.first();
}

QString StreamServer::getLocalIpV6()
{
    auto l = getLocalIps(true);
    for (const QString &a : l) {
        if (a.contains(':') && !a.startsWith("fe80")) return a;
    }
    for (const QString &a : l) if (a.contains(':')) return a;
    return QString();
}

bool StreamServer::startServer()
{
    if (!zlm_) {
        Q_EMIT errorOccurred(QStringLiteral("wrapper not initialized"));
        return false;
    }
    if (running_.load()) {
        qDebug() << "StreamServer: already running";
        return false;
    }
    stopRequested_.store(false);
    thr_ = std::make_unique<std::thread>(&StreamServer::threadMain, this);
    return true;
}

QStringList StreamServer::availableNetworkIps(bool includeV6)
{
    return getLocalIps(includeV6);
}


void StreamServer::stopServer()
{
    if (!running_.load() && (!thr_ || !thr_->joinable())) return;
    stopRequested_.store(true);
    if (zlm_) zlm_stop(zlm_);
    if (thr_ && thr_->joinable()) thr_->join();
    thr_.reset();
    running_.store(false);
    QMetaObject::invokeMethod(this, [this]() {
        Q_EMIT stopped();
        Q_EMIT runningChanged();
    }, Qt::QueuedConnection);
}

void StreamServer::setListenIp(const QString &ip)
{
    if (ip == listenIp_) return;
    listenIp_ = ip;
    if (zlm_) {
        // validate here as well: zlm_set_listen will also validate
        if (!listenIp_.isEmpty() && isLoopbackOrWildcard(listenIp_)) {
            listenIp_ = QString();
            Q_EMIT errorOccurred(tr("Refusing to set loopback/wildcard listen IP: %1").arg(listenIp_));
            return;
        }

        else {

             listenIp_ = ip;
        }

        zlm_set_listen(zlm_, listenIp_.isEmpty() ? nullptr : listenIp_.toUtf8().constData(), static_cast<uint16_t>(port_));
    }
    QMetaObject::invokeMethod(this, [this]() { Q_EMIT listenIpChanged(); }, Qt::QueuedConnection);
}

void StreamServer::setPort(int p)
{
    if (p <= 0) return;
    if (p == port_) return;
    port_ = p;
    if (zlm_) {
        zlm_set_listen(zlm_, listenIp_.isEmpty() ? nullptr : listenIp_.toUtf8().constData(), static_cast<uint16_t>(port_));
    }
    QMetaObject::invokeMethod(this, [this]() { Q_EMIT portChanged(); }, Qt::QueuedConnection);
}

void StreamServer::threadMain()
{
    // compute downloads paths
    QString downloads = downloadLocation();
    QString ini = computeIniPath();
    QString ssl = computeSslPath();

    // set Http::kRootPath (Downloads) BEFORE starting wrapper so config readers see it
    if (!downloads.isEmpty()) {
        // mINI expects std::string
        mINI::Instance()[Http::kRootPath] = std::string(downloads.toUtf8().constData());
        InfoL << "StreamServer: set Http::kRootPath = " << downloads.toStdString();
    } else {
        InfoL << "StreamServer: downloads path empty; not setting Http::kRootPath";
    }

    // Decide listen IP: prefer explicit override, else autodetect first non-loopback IPv4
    QString chosenIp = listenIp_;
    if (chosenIp.isEmpty()) {
        chosenIp = getLocalIp();
    }

    // If still empty or loopback/wildcard, refuse to start and require user to provide a valid LAN IP
    if (chosenIp.isEmpty() || isLoopbackOrWildcard(chosenIp)) {
        QString msg = tr("No non-loopback network IP found. Please set listen IP explicitly (e.g. your Wi-Fi IP).");
        QMetaObject::invokeMethod(this, [this, msg]() { Q_EMIT errorOccurred(msg); }, Qt::QueuedConnection);
        return;
    }

    // Provide IPv6 candidate (optional)
    QString chosenIpV6 = listenIpV6_;
    if (chosenIpV6.isEmpty()) chosenIpV6 = getLocalIpV6();

    // Apply listen IP to wrapper (validated non-loopback)
    if (zlm_) {
        int rc = zlm_set_listen(zlm_, chosenIp.toUtf8().constData(), static_cast<uint16_t>(port_));
        if (rc != 0) {
            QMetaObject::invokeMethod(this, [this, rc]() {
                Q_EMIT errorOccurred(QStringLiteral("zlm_set_listen failed: %1").arg(rc));
            }, Qt::QueuedConnection);
            return;
        }
    }

    // Start wrapper (this launches ZLMediaKit servers on wrapper thread)
    int rc = zlm_start_with_paths(zlm_, ini.toUtf8().constData(),downloads.toUtf8().constData(), ssl.toUtf8().constData());
    if (rc != 0) {
        QMetaObject::invokeMethod(this, [this, rc]() {
            Q_EMIT errorOccurred(QStringLiteral("zlm_start_with_paths failed code %1").arg(rc));
        }, Qt::QueuedConnection);
        return;
    }

    // Poll for running state (small timeout)
    int waited = 0;
    const int maxWait = 5000; // ms
    while (!stopRequested_.load() && waited < maxWait) {
        if (zlm_is_running(zlm_)) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        waited += 50;
    }

    if (!zlm_is_running(zlm_)) {
        QMetaObject::invokeMethod(this, [this]() {
            Q_EMIT errorOccurred(QStringLiteral("Wrapper did not report running state"));
        }, Qt::QueuedConnection);
        return;
    }

    // Update properties & Q_EMIT signals
    running_.store(true);
    listenIp_ = chosenIp;
    listenIpV6_ = chosenIpV6;
    QMetaObject::invokeMethod(this, [this]() {
        Q_EMIT started();
        Q_EMIT runningChanged();
        Q_EMIT listenIpChanged();
        Q_EMIT listenIpV6Changed();
    }, Qt::QueuedConnection);

    // Main loop: keep thread alive until stop requested
    while (!stopRequested_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Request wrapper stop and wait (wrapper joins its internal thread)
    if (zlm_) zlm_stop(zlm_);

    running_.store(false);
    QMetaObject::invokeMethod(this, [this]() {
        Q_EMIT stopped();
        Q_EMIT runningChanged();
    }, Qt::QueuedConnection);
}
