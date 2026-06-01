#include "AndroidService.h"

#if defined(Q_OS_ANDROID)

#include "DatabaseManager.h"
#include "SettingsManager.h"


#include "NotificationManager.h"
#include "utils_log.h"

#include <QtCore/private/qandroidextras_p.h>
#include <QCoreApplication>
#include <QDateTime>
#include <QTimer>
#include <QDebug>

/* ************************************************************************** */

AndroidService::AndroidService(QObject *parent) : QObject(parent)
{
    DatabaseManager::getInstance();

    m_settingsManager = SettingsManager::getInstance();

    //m_notificationManager = NotificationManager::getInstance(); // DEBUG
    //m_notificationManager->setNotification("AndroidService starting", QDateTime::currentDateTime().toString());

    // Configure update timer
    connect(&m_workTimer, &QTimer::timeout, this, &AndroidService::gotowork);
    setWorkTimer(5);
}

AndroidService::~AndroidService()
{
    //
}

/* ************************************************************************** */

void AndroidService::setWorkTimer(int workInterval_mins)
{
    m_workTimer.setInterval(workInterval_mins*60*1000);
    m_workTimer.start();
}

void AndroidService::gotowork()
{
    //m_notificationManager = NotificationManager::getInstance(); // DEBUG
    //m_notificationManager->setNotification("AndroidService gotowork", QDateTime::currentDateTime().toString());

    // Reload settings, user might have changed them
  //  m_settingsManager->reloadSettings();

    // Is the background service enabled?
  /*  if (m_settingsManager->getSysTray())
    {
        // Do something essential from the software core functions here
    } */

    // Restart timer
    setWorkTimer(5);
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
