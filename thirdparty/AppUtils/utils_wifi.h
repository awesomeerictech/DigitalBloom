#ifndef UTILS_WIFI_H
#define UTILS_WIFI_H
/* ************************************************************************** */

#include <QObject>
#include <QString>

/* ************************************************************************** */

/*!
 * \brief The UtilsWiFi class
 *
 * Android need the "ACCESS_WIFI_STATE" and "ACCESS_FINE_LOCATION" manifest permission.
 * iOS need the "NSLocationWhenInUseUsageDescription" plist key and "Wifi Info" capability.
 */
class UtilsWiFi: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentSSID READ getCurrentSSID NOTIFY wifiChanged)
    Q_PROPERTY(bool permissionOS READ hasPermissionOS NOTIFY permissionsChanged)

    QString m_currentSSID;

    bool m_permOS = false;

    void refreshWiFi_internal();

    // Singleton
    static UtilsWiFi *instance;
    UtilsWiFi();
    ~UtilsWiFi();

Q_SIGNALS:
    void wifiChanged();
    void permissionsChanged();

private Q_SLOTS:
    void requestLocationPermissions_results();

public:
    static UtilsWiFi *getInstance();

    Q_INVOKABLE void refreshWiFi();
    QString getCurrentSSID() { return m_currentSSID; }

    Q_INVOKABLE bool checkLocationPermissions();
    Q_INVOKABLE void requestLocationPermissions();
    bool hasPermissionOS() const { return m_permOS; }
};

/* ************************************************************************** */
#endif // UTILS_WIFI_H
