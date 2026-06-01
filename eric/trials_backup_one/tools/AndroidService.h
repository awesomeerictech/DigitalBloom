#ifndef ANDROIDSERVICE_H
#define ANDROIDSERVICE_H

#include <QObject>

/*
 * AndroidService
 *
 * Lightweight cross-platform wrapper used by the app to:
 *  - register the Java BroadcastReceiver at runtime
 *  - request start/stop of the Qt-managed Android foreground service (DigitalBloomService)
 *
 * On non-Android platforms these methods are no-ops (they simply log via qDebug).
 *
 * The heavy lifting is done in AndroidServiceJNI.cpp (native->Java and JNI glue).
 */
class AndroidService : public QObject
{
    Q_OBJECT
public:
    explicit AndroidService(QObject* parent = nullptr);
    ~AndroidService();

    // Register the Java BroadcastReceiver (dynamically) so system or custom broadcasts control the service.
    Q_INVOKABLE void service_registerCommService();

    // Request Java/QtService to start the foreground service (calls into JNI helpers)
    Q_INVOKABLE void service_start();

    // Request Java/QtService to stop the foreground service (calls into JNI helpers)
    Q_INVOKABLE void service_stop();
};

#endif // ANDROIDSERVICE_H
