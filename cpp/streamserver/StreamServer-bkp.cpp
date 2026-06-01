#include "StreamServer.hpp"

#include <QStandardPaths>
#include <QDir>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QFile>
#include <QMetaObject>
#include <QDebug>

extern "C" {
#include "zlm_wrapper_api.h"
}

StreamServer::StreamServer(QObject *parent)
    : QObject(parent)
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
    QString loc = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
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
    return dl + QStringLiteral("zlmediakit.ini");
}

QString StreamServer::computeSslPath()
{
    QString dl = downloadLocation();
    return dl + QStringLiteral("ssl.pem");
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
    // Do not block UI: threadMain will Q_EMIT started()/error via queued signals
    return true;
}

void StreamServer::stopServer()
{
    if (!running_.load() && (!thr_ || !thr_->joinable())) return;
    stopRequested_.store(true);
    // Call wrapper stop to request shutdown
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

void StreamServer::threadMain()
{
    // compute paths
    QString ini = computeIniPath();
    QString ssl = computeSslPath();

    // instruct wrapper about listen IP/port before start
    if (!listenIp_.isEmpty() || port_ > 0) {
        zlm_set_listen(zlm_,
                       listenIp_.isEmpty() ? nullptr : listenIp_.toUtf8().constData(),
                       static_cast<uint16_t>(port_));
    }

    // start wrapper (this launches wrapper background thread)
    int rc = zlm_start_with_paths(zlm_, ini.toUtf8().constData(), ssl.toUtf8().constData());
    if (rc != 0) {
        QMetaObject::invokeMethod(this, [this, rc]() {
            Q_EMIT errorOccurred(QStringLiteral("zlm_start_with_paths failed code %1").arg(rc));
        }, Qt::QueuedConnection);
        return;
    }

    // wait for wrapper to report running (poll with timeout)
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

    running_.store(true);
    QMetaObject::invokeMethod(this, [this]() {
        Q_EMIT started();
        Q_EMIT runningChanged();
    }, Qt::QueuedConnection);

    // main loop: wait until stopRequested_
    while (!stopRequested_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // request wrapper stop and join wrapper's thread inside wrapper
    zlm_stop(zlm_);

    running_.store(false);
    QMetaObject::invokeMethod(this, [this]() {
        Q_EMIT stopped();
        Q_EMIT runningChanged();
    }, Qt::QueuedConnection);
}
