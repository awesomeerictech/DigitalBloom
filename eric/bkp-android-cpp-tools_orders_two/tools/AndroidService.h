#ifndef ANDROID_SERVICE_H
#define ANDROID_SERVICE_H
/* ************************************************************************** */

#include <QtGlobal>
#include <QObject>
#include <QTimer>

#if defined(Q_OS_ANDROID)


class SettingsManager;
class NotificationManager;
class QQmlContext;
class ServerManager;

/* ************************************************************************** */

/*!
 * \brief The AndroidService class
 */
class AndroidService: public QObject
{
    Q_OBJECT

    QTimer m_workTimer;
    //void setWorkTimer(int workInterval_duration = 5);
    void setWorkTimer(int workInterval_duration = 1000);

    
    SettingsManager *m_settingsManager = nullptr;
    NotificationManager *m_notificationManager = nullptr;

private Q_SLOTS:
    void gotowork();

public:
    AndroidService(QObject *parent = nullptr);
    ~AndroidService();

    static void service_start();
    static void service_stop();
    static void service_registerCommService();
    void setcontext(QQmlContext* context);

private:
    QQmlContext* thecontext_ = nullptr;
    ServerManager* mgr;

};

/* ************************************************************************** */
#endif // Q_OS_ANDROID
#endif // ANDROID_SERVICE_H
