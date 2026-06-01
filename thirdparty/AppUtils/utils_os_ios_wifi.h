#ifndef UTILS_OS_IOS_WIFI_H
#define UTILS_OS_IOS_WIFI_H

#include <QtGlobal>
#include <QString>

#if defined(Q_OS_IOS)
/* ************************************************************************** */

/*!
 * \brief iOS WiFi info
 */
class UtilsIOSWiFi
{
public:
    static QString getWifiSSID();
};

/* ************************************************************************** */
#endif // Q_OS_IOS
#endif // UTILS_OS_IOS_WIFI_H
