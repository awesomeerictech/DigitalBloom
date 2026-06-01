#include "AndroidService.h"

#if defined(Q_OS_ANDROID)

#include "DatabaseManager.h"
#include "SettingsManager.h"
#include "ServerManager.h" // Assumed header for ServerManager singleton


#include "NotificationManager.h"
#include "utils_log.h"

#include <QtCore/private/qandroidextras_p.h>
#include <QCoreApplication>
#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include <QtQml>

/* ************************************************************************** */

AndroidService::AndroidService(QObject *parent) : QObject(parent)
{
    DatabaseManager::getInstance();
    mgr = ServerManager::instance();

    m_settingsManager = SettingsManager::getInstance();

    m_notificationManager = NotificationManager::getInstance(); // DEBUG
    m_notificationManager->setNotification("DigitalBloom starting", QDateTime::currentDateTime().toString(),0,101);

    // Configure update timer
    connect(&m_workTimer, &QTimer::timeout, this, &AndroidService::gotowork);
    setWorkTimer(1000);
}

AndroidService::~AndroidService()
{
    //
}

/* ************************************************************************** */

void AndroidService::setWorkTimer(int workInterval_duration)
{
   // m_workTimer.setInterval(workInterval_durations*60*1000);
    m_workTimer.setInterval(workInterval_duration);

    m_workTimer.start();
}

void AndroidService::setcontext(QQmlContext* context) {
    thecontext_ = context;

}

void AndroidService::gotowork()
{
    m_notificationManager = NotificationManager::getInstance(); // DEBUG
    m_notificationManager->setNotification("DigitalBloom Running", QDateTime::currentDateTime().toString(),1,102);

    // Reload settings, user might have changed them
   m_settingsManager->reloadSettings();

    // Is the background service enabled?
    if (m_settingsManager->getSysTray())
    {
        // Do something essential from the software core functions here
    }

    if(mgr && thecontext_){
        ServerManager::registerQmlTypes();
        mgr->mycontext(thecontext_);
        qInfo() << "Starting DigitalBloom Service in the Foreground" << Qt::endl;
        mgr->startAllServers();

    }

    // Restart timer
     setWorkTimer(5*60*1000);
}

/* ************************************************************************** */

void AndroidService::service_start()
{
    QJniObject::callStaticMethod<void>("org.stlltd.digitalbloom.DigitalBloomService",
                                       "serviceStart",
                                       "(Landroid/content/Context;)V",
                                       QNativeInterface::QAndroidApplication::context());
}

void AndroidService::service_stop()
{
    QJniObject::callStaticMethod<void>("org.stlltd.digitalbloom.DigitalBloomService",
                                       "serviceStop", "(Landroid/content/Context;)V",
                                       QNativeInterface::QAndroidApplication::context());
}

void AndroidService::service_registerCommService()
{
    QJniEnvironment env;
    jclass javaClass = env.findClass("org/stlltd/digitalbloom/ActivityUtils");
    QJniObject classObject(javaClass);

    classObject.callMethod<void>("registerServiceBroadcastReceiver",
                                 "(Landroid/content/Context;)V",
                                 QNativeInterface::QAndroidApplication::context());
}

/* ************************************************************************** */
#endif // Q_OS_ANDROID
