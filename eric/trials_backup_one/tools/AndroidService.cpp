#include "AndroidService.h"
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#endif

// These functions are implemented in AndroidServiceJNI.cpp
// (they call Java helpers such as ActivityUtils.registerServiceBroadcastReceiver
//  and DigitalBloomService.startFromContext / stopFromContext)
extern void registerServiceBroadcastReceiverFromNative();
extern void startQtServiceFromNative();
extern void stopQtServiceFromNative();

AndroidService::AndroidService(QObject* parent)
    : QObject(parent)
{
    qInfo() << "AndroidService constructed";
}

AndroidService::~AndroidService()
{
    qInfo() << "AndroidService destructed";
}

void AndroidService::service_registerCommService()
{
#ifdef Q_OS_ANDROID
    qInfo() << "AndroidService::service_registerCommService() - registering Java BroadcastReceiver";
    // Delegates to JNI helper which will call:
    // ActivityUtils.registerServiceBroadcastReceiver(context)
    registerServiceBroadcastReceiverFromNative();
#else
    qInfo() << "AndroidService::service_registerCommService() - not Android, no-op";
#endif
}

void AndroidService::service_start()
{
#ifdef Q_OS_ANDROID
    qInfo() << "AndroidService::service_start() - asking Java to start QtService (foreground)";
    // Delegates to JNI helper which will call:
    // DigitalBloomService.startFromContext(context)
    startQtServiceFromNative();
#else
    qInfo() << "AndroidService::service_start() - not Android, no-op";
#endif
}

void AndroidService::service_stop()
{
#ifdef Q_OS_ANDROID
    qInfo() << "AndroidService::service_stop() - asking Java to stop QtService (foreground)";
    // Delegates to JNI helper which will call:
    // DigitalBloomService.stopFromContext(context)
    stopQtServiceFromNative();
#else
    qInfo() << "AndroidService::service_stop() - not Android, no-op";
#endif
}
