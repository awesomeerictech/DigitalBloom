// AndroidServiceJNI.cpp
#include <jni.h>
#include <QDebug>
#include <QCoreApplication>
#include <QMetaObject>
#include <QtCore/private/qandroidextras_p.h>
#include <QtGlobal>
#include "ServerManager.h"

extern "C" JNIEXPORT void JNICALL
Java_org_stlltd_digitalbloom_DigitalBloomService_nativeOnServiceCreate(JNIEnv* env, jclass cls)
{
    Q_UNUSED(env);
    Q_UNUSED(cls);
    qInfo() << "JNI: nativeOnServiceCreate called";

    // Schedule start on Qt main thread (safe cross-thread)
    QMetaObject::invokeMethod(QCoreApplication::instance(), []() {
        ServerManager* m = serverManagerInstance();
        if (m) {
            m->startAllServers();
        } else {
            qWarning() << "ServerManager instance missing in nativeOnServiceCreate";
        }
    }, Qt::QueuedConnection);
}

extern "C" JNIEXPORT void JNICALL
Java_org_stlltd_digitalbloom_DigitalBloomService_nativeOnServiceDestroy(JNIEnv* env, jclass cls)
{
    Q_UNUSED(env);
    Q_UNUSED(cls);
    qInfo() << "JNI: nativeOnServiceDestroy called";

    QMetaObject::invokeMethod(QCoreApplication::instance(), []() {
        ServerManager* m = serverManagerInstance();
        if (m) {
            m->stopAllServers();
        } else {
            qWarning() << "ServerManager instance missing in nativeOnServiceDestroy";
        }
    }, Qt::QueuedConnection);
}

/**
 * Optional native helpers that your existing AndroidService.cpp calls (WatchFlower style).
 * These helpers call Java ActivityUtils.registerServiceBroadcastReceiver(context)
 * and the QtService start/stop helpers.
 *
 * Example usage from C++:
 *   registerServiceBroadcastReceiverFromNative();
 *   startQtServiceFromNative();  // starts the Qt service (DigitalBloomService)
 *   stopQtServiceFromNative();
 */

void registerServiceBroadcastReceiverFromNative()
{
    // static call: ActivityUtils.registerServiceBroadcastReceiver(Context)
    QJniEnvironment env;
    // get application context via Qt private helper (qandroidextras_p.h)
    jobject context = QNativeInterface::QAndroidApplication::context().object();
    if (!context) {
        qWarning() << "registerServiceBroadcastReceiverFromNative: no Android context";
        return;
    }

    // Call static method
    QJniObject::callStaticMethod<void>(
        "org/stlltd/digitalbloom/ActivityUtils",
        "registerServiceBroadcastReceiver",
        "(Landroid/content/Context;)V",
        context
    );
    qInfo() << "Called ActivityUtils.registerServiceBroadcastReceiver";
}

void startQtServiceFromNative()
{
    // Use QtService.start(Context, ServiceClass)
    jobject context = QNativeInterface::QAndroidApplication::context().object();
    if (!context) {
        qWarning() << "startQtServiceFromNative: no Android context";
        return;
    }

    // Call: QtService.start(context, DigitalBloomService.class)
    // QtService.start is a static Java method in org.qtproject.qt.android.bindings.QtService
    // But simpler: call DigitalBloomService.startFromContext(context) which we implemented in Java
    QJniObject::callStaticMethod<void>(
        "org/stlltd/digitalbloom/DigitalBloomService",
        "startFromContext",
        "(Landroid/content/Context;)V",
        context
    );
    qInfo() << "Requested DigitalBloomService.startFromContext()";
}

void stopQtServiceFromNative()
{
    jobject context = QNativeInterface::QAndroidApplication::context().object();
    if (!context) {
        qWarning() << "stopQtServiceFromNative: no Android context";
        return;
    }

    QJniObject::callStaticMethod<void>(
        "org/stlltd/digitalbloom/DigitalBloomService",
        "stopFromContext",
        "(Landroid/content/Context;)V",
        context
    );
    qInfo() << "Requested DigitalBloomService.stopFromContext()";
}
