#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H
/* ************************************************************************** */

#include <QObject>
#include <QString>

#if defined(Q_OS_IOS)
#include "utils_os_ios_notif.h"
#endif

/* ************************************************************************** */

/*!
 * \brief The NotificationManager class
 */
class NotificationManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString notification READ getNotification WRITE setNotificationShort NOTIFY notificationChanged)
    Q_PROPERTY(bool permissionOS READ hasPermissionOS NOTIFY permissionsChanged)

    QString m_title;
    QString m_message;
    int m_channel = 0;
    int m_notificationId = 1; // <--- ADDED: New member for unique notification ID

    bool m_permOS = false;
    bool hasPermissionOS() const { return m_permOS; }

#if defined(Q_OS_IOS)
    //UtilsIOSNotifications m_iosnotifier;
#endif

    // Singleton
    static NotificationManager *instance;
    NotificationManager();
    ~NotificationManager();

private Q_SLOTS:
    void updateNotificationAndroid();
    void updateNotificationIOS();
    void updateNotificationDesktop();

Q_SIGNALS:
    void notificationChanged();
    void permissionsChanged();

public:
    static NotificationManager *getInstance();

    Q_INVOKABLE bool checkNotificationPermissions();
    Q_INVOKABLE bool requestNotificationPermissions();

    QString getNotification() const { return m_message; }
    // --- UPDATED: Added 'int notificationId = 1' ---
    void setNotification(const QString &title, const QString &message, int channel = 0, int notificationId = 101);
    void setNotificationShort(const QString &message);
};

/* ************************************************************************** */
#endif // NOTIFICATION_MANAGER_H
