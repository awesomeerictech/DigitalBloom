// AndroidService.cpp
#include "AndroidService.h"
#include <QJniObject>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include <QDebug>

void AndroidService::serviceStart(int serverId, const QString &iniPath, const QString &sslPath) {
#ifdef Q_OS_ANDROID
    QAndroidJniObject ctx = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                       "activity",
                                                                       "()Landroid/app/Activity;");
    if (!ctx.isValid()) {
        qWarning() << "AndroidService::serviceStart: failed to get activity/context";
        return;
    }

    QAndroidJniObject jIni = QAndroidJniObject::fromString(iniPath);
    QAndroidJniObject jSsl = QAndroidJniObject::fromString(sslPath);

    QAndroidJniObject::callStaticMethod<void>(
        "org.stlltd.digitalbloom.DigitalBloomService",
        "serviceStart",
        "(Landroid/content/Context;ILjava/lang/String;Ljava/lang/String;)V",
        ctx.object<jobject>(),
        serverId,
        jIni.object<jstring>(),
        jSsl.object<jstring>()
    );
#else
    Q_UNUSED(serverId);
    Q_UNUSED(iniPath);
    Q_UNUSED(sslPath);
    qWarning() << "AndroidService::serviceStart: not running on Android";
#endif
}

void AndroidService::serviceStop(int serverId) {
#ifdef Q_OS_ANDROID
    QAndroidJniObject ctx = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                       "activity",
                                                                       "()Landroid/app/Activity;");
    if (!ctx.isValid()) {
        qWarning() << "AndroidService::serviceStop: failed to get activity/context";
        return;
    }

    QAndroidJniObject::callStaticMethod<void>(
        "org.stlltd.digitalbloom.DigitalBloomService",
        "serviceStop",
        "(Landroid/content/Context;I)V",
        ctx.object<jobject>(),
        serverId
    );
#else
    Q_UNUSED(serverId);
    qWarning() << "AndroidService::serviceStop: not running on Android";
#endif
}
