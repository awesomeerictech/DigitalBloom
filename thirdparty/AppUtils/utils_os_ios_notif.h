#ifndef UTILS_OS_IOS_NOTIF_H
#define UTILS_OS_IOS_NOTIF_H

#include <QtGlobal>
#include <QString>

#if defined(Q_OS_IOS)
/* ************************************************************************** */

/*!
 * \brief iOS notifications
 */
class UtilsIOSNotifications
{
    void *m_notifdelegate = nullptr;

public:
    UtilsIOSNotifications();

    /*!
     * \return True if notification permission has been previously obtained.
     */
    static bool checkPermission_notification();

    /*!
     * \return True if notification permission has been explicitly obtained.
     */
    static bool getPermission_notification();

    /*!
     * \brief notify
     * \param title
     * \param message
     * \param channel
     * \return
     */
    bool notify(const QString &title, const QString &message, const int channel);
};

/* ************************************************************************** */
#endif // Q_OS_IOS
#endif // UTILS_OS_IOS_NOTIF_H
