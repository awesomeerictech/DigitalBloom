#include "SettingsManager.h"
#include "SystrayManager.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QLocale>
#include <QDir>
#include <QDebug>



/* ************************************************************************** */

SettingsManager *SettingsManager::instance = nullptr;

SettingsManager *SettingsManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new SettingsManager();
        return instance;
    }

    return instance;
}

SettingsManager::SettingsManager()
{
    readSettings();
}

SettingsManager::~SettingsManager()
{
    //
}

QString SettingsManager::getSettingsDirectory()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    return settings.fileName();
}

bool SettingsManager::saveSettings()
{
    // settings file?
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString internalPath = settings.fileName();

    // save dir
    QString externalPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/digitalbloom";
    QDir externalDir(externalPath);
    if (!externalDir.exists()) externalDir.mkpath(externalPath);

    return QFile::copy(internalPath,
                       externalPath + "/digitalbloom_settings_" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+ ".ini");
}

bool SettingsManager::restoreSettings()
{
    return false;
}

void SettingsManager::reloadSettings()
{
    readSettings();
}


/* ************************************************************************** */
/* ************************************************************************** */

bool SettingsManager::readSettings()
{
    bool status = false;

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    if (settings.status() == QSettings::NoError)
    {
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS) || defined(Q_OS_WINDOWS)
        if (settings.contains("ApplicationWindow/x"))
            m_appPosition.setWidth(settings.value("ApplicationWindow/x").toInt());
        if (settings.contains("ApplicationWindow/y"))
            m_appPosition.setHeight(settings.value("ApplicationWindow/y").toInt());
        if (settings.contains("ApplicationWindow/width"))
            m_appSize.setWidth(settings.value("ApplicationWindow/width").toInt());
        if (settings.contains("ApplicationWindow/height"))
            m_appSize.setHeight(settings.value("ApplicationWindow/height").toInt());
        if (settings.contains("ApplicationWindow/visibility"))
            m_appVisibility = settings.value("ApplicationWindow/visibility").toUInt();

        if (m_appPosition.width() > 8192) m_appPosition.setWidth(100);
        if (m_appPosition.height() > 8192) m_appPosition.setHeight(100);
        if (m_appSize.width() > 8192) m_appSize.setWidth(1920);
        if (m_appSize.height() > 8192) m_appSize.setHeight(1080);
        if (m_appVisibility < 1 || m_appVisibility > 5) m_appVisibility = 1;
#endif

        if (settings.contains("settings/appTheme"))
            m_appTheme = settings.value("settings/appTheme").toString();

        if (settings.contains("settings/appThemeAuto"))
            m_appThemeAuto = settings.value("settings/appThemeAuto").toBool();

        ////

        if (settings.contains("settings/defaultTab"))
            m_defaultTab = settings.value("settings/defaultTab").toString();

        if (settings.contains("settings/backendWriter"))
            m_backendWriter = settings.value("settings/backendWriter").toString();

        if (settings.contains("settings/formatsEnabled_zxingcpp"))
            m_formatsEnabled_zxingcpp = settings.value("settings/formatsEnabled_zxingcpp").toUInt();
        if (settings.contains("settings/formatsEnabled_qzxing"))
            m_formatsEnabled_qzxing = settings.value("settings/formatsEnabled_qzxing").toUInt();

        ////

        if (settings.contains("settings/saveBarcodes"))
            m_save_barcodes = settings.value("settings/saveBarcodes").toBool();
        if (settings.contains("settings/saveCamera"))
            m_save_camera = settings.value("settings/saveCamera").toBool();
        if (settings.contains("settings/saveGPS"))
            m_save_gps = settings.value("settings/saveGPS").toBool();

        ////

        if (settings.contains("settings/showDebug"))
            m_showDebug = settings.value("settings/showDebug").toBool();

        if (settings.contains("settings/scanFullresolution"))
            m_scan_fullresolution = settings.value("settings/scanFullresolution").toBool();
        if (settings.contains("settings/scanFullscreen"))
            m_scan_fullscreen = settings.value("settings/scanFullscreen").toBool();
        if (settings.contains("settings/scanTryHarder"))
            m_scan_tryHarder = settings.value("settings/scanTryHarder").toBool();
        if (settings.contains("settings/scanTryRotate"))
            m_scan_tryRotate = settings.value("settings/scanTryRotate").toBool();
        if (settings.contains("settings/scanTryInvert"))
            m_scan_tryInvert = settings.value("settings/scanTryInvert").toBool();
        if (settings.contains("settings/scanTryDownscale"))
            m_scan_tryDownscale = settings.value("settings/scanTryDownscale").toBool();

        if (settings.contains("settings/appLanguage"))
            m_appLanguage = settings.value("settings/appLanguage").toString();

        if (settings.contains("settings/bluetoothControl"))
            m_bluetoothControl = settings.value("settings/bluetoothControl").toBool();

        if (settings.contains("settings/bluetoothLimitScanningRange"))
            m_bluetoothLimitScanningRange = settings.value("settings/bluetoothLimitScanningRange").toBool();

        if (settings.contains("settings/bluetoothSimUpdates"))
            m_bluetoothSimUpdates = settings.value("settings/bluetoothSimUpdates").toUInt();
        else
        {
#if defined(Q_OS_ANDROID)
            // too many weak devices on Android...
            m_bluetoothSimUpdates = 2;
#elif defined(Q_OS_IOS)
            // iOS is better
            m_bluetoothSimUpdates = 3;
#else
            // desktops are usually good with simultaneous updates
            m_bluetoothSimUpdates = 4;
#endif
        }

        if (settings.contains("settings/startMinimized"))
            m_startMinimized = settings.value("settings/startMinimized").toBool();

        if (settings.contains("settings/trayEnabled"))
            m_systrayEnabled = settings.value("settings/trayEnabled").toBool();

        if (settings.contains("settings/notifsEnabled"))
            m_notificationsEnabled = settings.value("settings/notifsEnabled").toBool();
        if (settings.contains("settings/notifsBatt"))
            m_notif_batt = settings.value("settings/notifsBatt").toBool();
        if (settings.contains("settings/notifsWater"))
            m_notif_water = settings.value("settings/notifsWater").toBool();
        if (settings.contains("settings/notifsSubzero"))
            m_notif_subzero = settings.value("settings/notifsSubzero").toBool();
        if (settings.contains("settings/notifsEnv"))
            m_notif_env = settings.value("settings/notifsEnv").toBool();

        if (settings.contains("settings/updateIntervalBackground"))
            m_updateIntervalBackground = settings.value("settings/updateIntervalBackground").toInt();

        if (settings.contains("settings/updateIntervalPlant"))
            m_updateIntervalPlant = settings.value("settings/updateIntervalPlant").toInt();

        if (settings.contains("settings/updateIntervalThermo"))
            m_updateIntervalThermometer = settings.value("settings/updateIntervalThermo").toInt();

        if (settings.contains("settings/updateIntervalEnv"))
            m_updateIntervalEnvironmental = settings.value("settings/updateIntervalEnv").toInt();

        if (settings.contains("settings/tempUnit"))
            m_tempUnit = settings.value("settings/tempUnit").toString();
        else
        {
            // If we have no measurement system saved, use system's one
            // TODO: i18n may not have been set yet?
            QLocale lo;
            if (lo.measurementSystem() == QLocale::MetricSystem)
                m_tempUnit = "C";
            else
                m_tempUnit = "F";
        }

        if (settings.contains("settings/graphHistory"))
            m_graphHistogram = settings.value("settings/graphHistory").toString();
        if (settings.contains("settings/graphThermometer"))
            m_graphThermometer = settings.value("settings/graphThermometer").toString();
        if (settings.contains("settings/graphAioDays"))
            m_graphAioDays = settings.value("settings/graphAioDays").toInt();
        if (settings.contains("settings/graphAioShowDots"))
            m_graphAioShowDots = settings.value("settings/graphAioShowDots").toBool();

        if (settings.contains("settings/bigIndicator"))
            m_bigIndicator = settings.value("settings/bigIndicator").toBool();

        if (settings.contains("settings/dynaScale"))
            m_dynaScale = settings.value("settings/dynaScale").toBool();

        if (settings.contains("settings/splitView"))
            m_splitView = settings.value("settings/splitView").toBool();
        if (settings.contains("settings/orderBy"))
            m_orderBy = settings.value("settings/orderBy").toString();

        if (settings.contains("settings/location"))
            m_location = settings.value("settings/location").toBool();
        if (settings.contains("settings/locationLatitude"))
            m_location_latitude = settings.value("settings/locationLatitude").toFloat();
        if (settings.contains("settings/locationLongitude"))
            m_location_longitude = settings.value("settings/locationLongitude").toFloat();
        if (settings.contains("settings/sunandmoon"))
            m_sunandmoon = settings.value("settings/sunandmoon").toBool();

        if (settings.contains("database/enabled"))
            m_mysql = settings.value("database/enabled").toBool();
        if (settings.contains("database/host"))
            m_mysqlHost = settings.value("database/host").toString();
        if (settings.contains("database/port"))
            m_mysqlPort = settings.value("database/port").toInt();
        if (settings.contains("database/name"))
            m_mysqlName = settings.value("database/name").toString();
        if (settings.contains("database/user"))
            m_mysqlUser = settings.value("database/user").toString();
        if (settings.contains("database/password"))
            m_mysqlPassword = settings.value("database/password").toString();

        if (settings.contains("mqtt/enabled"))
            m_mqtt = settings.value("mqtt/enabled").toBool();
        if (settings.contains("mqtt/host"))
            m_mqttHost = settings.value("mqtt/host").toString();
        if (settings.contains("mqtt/port"))
            m_mqttPort = settings.value("mqtt/port").toInt();
        if (settings.contains("mqtt/name"))
            m_mqttName = settings.value("mqtt/name").toString();
        if (settings.contains("mqtt/user"))
            m_mqttUser = settings.value("mqtt/user").toString();
        if (settings.contains("mqtt/password"))
            m_mqttPassword = settings.value("mqtt/password").toString();

        status = true;
    }
    else
    {
        qWarning() << "SettingsManager::readSettings() error:" << settings.status();
    }

    return status;
}


/* ************************************************************************** */

bool SettingsManager::writeSettings()
{
    bool status = false;

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    if (settings.isWritable())
    {
        settings.setValue("settings/appTheme", m_appTheme);
        settings.setValue("settings/appThemeAuto", m_appThemeAuto);

        settings.setValue("settings/defaultTab", m_defaultTab);
        settings.setValue("settings/backendWriter", m_backendWriter);
        settings.setValue("settings/formatsEnabled_zxingcpp", m_formatsEnabled_zxingcpp);
        settings.setValue("settings/formatsEnabled_qzxing", m_formatsEnabled_qzxing);

        settings.setValue("settings/saveBarcodes", m_save_barcodes);
        settings.setValue("settings/saveCamera", m_save_camera);
        settings.setValue("settings/saveGPS", m_save_gps);

        settings.setValue("settings/showDebug", m_showDebug);
        settings.setValue("settings/scanFullresolution", m_scan_fullresolution);
        settings.setValue("settings/scanFullscreen", m_scan_fullscreen);
        settings.setValue("settings/scanTryHarder", m_scan_tryHarder);
        settings.setValue("settings/scanTryRotate", m_scan_tryRotate);
        settings.setValue("settings/scanTryInvert", m_scan_tryInvert);
        settings.setValue("settings/scanTryDownscale", m_scan_tryDownscale);

        // new settings

        settings.setValue("settings/appLanguage", m_appLanguage);

        settings.setValue("settings/bluetoothControl", m_bluetoothControl);
        settings.setValue("settings/bluetoothLimitScanningRange", m_bluetoothLimitScanningRange);
        settings.setValue("settings/bluetoothSimUpdates", m_bluetoothSimUpdates);
        settings.setValue("settings/startMinimized", m_startMinimized);
        settings.setValue("settings/trayEnabled", m_systrayEnabled);
        settings.setValue("settings/notifsEnabled", m_notificationsEnabled);
        settings.setValue("settings/notifsBatt", m_notif_batt);
        settings.setValue("settings/notifsWater", m_notif_water);
        settings.setValue("settings/notifsSubzero", m_notif_subzero);
        settings.setValue("settings/notifsEnv", m_notif_env);
        //settings.setValue("settings/updateIntervalBackground", m_updateIntervalBackground);
        settings.setValue("settings/updateIntervalPlant", m_updateIntervalPlant);
        settings.setValue("settings/updateIntervalThermo", m_updateIntervalThermometer);
        //settings.setValue("settings/updateIntervalEnv", m_updateIntervalEnvironmental);
        settings.setValue("settings/graphHistory", m_graphHistogram);
        settings.setValue("settings/graphThermometer", m_graphThermometer);
        settings.setValue("settings/graphAioDays", m_graphAioDays);
        settings.setValue("settings/graphAioShowDots", m_graphAioShowDots);
        settings.setValue("settings/bigIndicator", m_bigIndicator);
        settings.setValue("settings/tempUnit", m_tempUnit);
        settings.setValue("settings/dynaScale", m_dynaScale);
        settings.setValue("settings/splitView", m_splitView);
        settings.setValue("settings/orderBy", m_orderBy);

        settings.setValue("settings/location", m_location_latitude);
        settings.setValue("settings/locationLatitude", m_location_latitude);
        settings.setValue("settings/locationLongitude", m_location_longitude);
        settings.setValue("settings/sunandmoon", m_sunandmoon);

        settings.setValue("database/enabled", m_mysql);
        settings.setValue("database/host", m_mysqlHost);
        settings.setValue("database/port", m_mysqlPort);
        settings.setValue("database/name", m_mysqlName);
        settings.setValue("database/user", m_mysqlUser);
        settings.setValue("database/password", m_mysqlPassword);

        settings.setValue("mqtt/enabled", m_mqtt);
        settings.setValue("mqtt/host", m_mqttHost);
        settings.setValue("mqtt/port", m_mqttPort);
        settings.setValue("mqtt/name", m_mqttName);
        settings.setValue("mqtt/user", m_mqttUser);
        settings.setValue("mqtt/password", m_mqttPassword);

        if (settings.status() == QSettings::NoError)
        {
            status = true;
        }
        else
        {
            qWarning() << "SettingsManager::writeSettings() error:" << settings.status();
        }
    }
    else
    {
        qWarning() << "SettingsManager::writeSettings() error: read only file?";
    }

    return status;
}

/* ************************************************************************** */

void SettingsManager::resetSettings()
{
    m_appTheme = "light";
    Q_EMIT appThemeChanged();
    m_appThemeAuto = false;
    Q_EMIT appThemeAutoChanged();

    m_appThemeCSD = false;
    Q_EMIT appThemeCSDChanged();
    m_appUnits = 0;
    Q_EMIT appUnitsChanged();
    m_appLanguage = "auto";
    Q_EMIT appLanguageChanged();

    m_startMinimized = false;
    Q_EMIT minimizedChanged();
    m_systrayEnabled = true;
    Q_EMIT systrayChanged();
    m_notificationsEnabled = true;
    m_notif_batt = true;
    m_notif_water = true;
    m_notif_subzero = true;
    m_notif_env = true;
    Q_EMIT notifsChanged();
    m_updateIntervalBackground = s_intervalBackgroundUpdate;
    Q_EMIT updateIntervalBackgroundChanged();
    m_updateIntervalPlant = s_intervalPlantUpdate;
    Q_EMIT updateIntervalPlantChanged();
    m_updateIntervalThermometer = s_intervalThermometerUpdate;
    Q_EMIT updateIntervalThermoChanged();
    m_updateIntervalEnvironmental = s_intervalEnvironmentalUpdate;
    Q_EMIT updateIntervalEnvChanged();

    m_bluetoothControl = false;
    Q_EMIT bluetoothControlChanged();
    m_bluetoothLimitScanningRange = false;
    Q_EMIT bluetoothLimitScanningRangeChanged();
    m_bluetoothSimUpdates = 2;
    Q_EMIT bluetoothSimUpdatesChanged();

    QLocale lo;
    if (lo.measurementSystem() == QLocale::MetricSystem)
        m_tempUnit = "C";
    else
        m_tempUnit = "F";
    Q_EMIT tempUnitChanged();

    m_graphHistogram = "monthly";
    Q_EMIT graphHistogramChanged();
    m_graphThermometer = "minmax";
    Q_EMIT graphThermometerChanged();
    m_graphAioDays = 14;
    m_graphAioShowDots = false;
    Q_EMIT graphAioChanged();

    m_orderBy = "model";
    Q_EMIT orderByChanged();
    m_splitView = false;
    Q_EMIT splitViewChanged();

    m_bigIndicator = false;
    Q_EMIT bigIndicatorChanged();
    m_dynaScale = false;
    Q_EMIT dynaScaleChanged();

    m_location = false;
    m_location_latitude = 0.f;
    m_location_longitude = 0.f;
    m_sunandmoon = false;

    m_mysql = false;
    m_mysqlHost = "";
    m_mysqlPort = 3306;
    m_mysqlName = "digitalbloom";
    m_mysqlUser = "digitalbloom";
    m_mysqlPassword = "digitalbloom";
    Q_EMIT mysqlChanged();

    m_mqtt = false;
    m_mqttHost = "";
    m_mqttPort = 1883;
    m_mqttName = "digitalbloom";
    m_mqttUser = "digitalbloom";
    m_mqttPassword = "digitalbloom";
    m_mqttTopics = "digitalbloom";
    Q_EMIT mqttChanged();

    m_defaultTab = "reader";
    m_backendWriter = "";
    m_formatsEnabled_zxingcpp = 0xffffffff; // ZXing::LinearCodes | ZXing::MatrixCodes;
    m_formatsEnabled_qzxing = 0xffffffff; // QZXing::LinearCodes | QZXing::MatrixCodes;

    m_save_barcodes = false;
    m_save_camera = false;
    m_save_gps = false;

    m_showDebug = false;
    m_scan_fullresolution = false;
    m_scan_fullscreen = false;
    m_scan_tryHarder = true;
    m_scan_tryRotate = true;
    m_scan_tryInvert = true;
    m_scan_tryDownscale = true;

    writeSettings();
}

/* ************************************************************************** */
/* ************************************************************************** */

void SettingsManager::setAppTheme(const QString &value)
{
    if (m_appTheme != value)
    {
        m_appTheme = value;
        Q_EMIT appThemeChanged();

        writeSettings();
    }
}

void SettingsManager::setAppThemeAuto(const bool value)
{
    if (m_appThemeAuto != value)
    {
        m_appThemeAuto = value;
        Q_EMIT appThemeAutoChanged();

        writeSettings();
    }
}

void SettingsManager::setAppThemeCSD(const bool value)
{
    if (m_appThemeCSD != value)
    {
        m_appThemeCSD = value;
        writeSettings();
        Q_EMIT appThemeCSDChanged();
    }
}

void SettingsManager::setAppUnits(const unsigned value)
{
    if (m_appUnits != value)
    {
        m_appUnits = value;
        writeSettings();
        Q_EMIT appUnitsChanged();
    }
}

void SettingsManager::setAppLanguage(const QString &value)
{
    if (m_appLanguage != value)
    {
        m_appLanguage = value;
        writeSettings();
        Q_EMIT appLanguageChanged();
    }
}

void SettingsManager::setMinimized(const bool value)
{
    if (m_startMinimized != value)
    {
        m_startMinimized = value;
        writeSettings();
        Q_EMIT minimizedChanged();
    }
}

void SettingsManager::setNotifs(const bool value)
{
    if (m_notificationsEnabled != value)
    {
        m_notificationsEnabled = value;
        writeSettings();
        Q_EMIT notifsChanged();
    }
}

void SettingsManager::setNotifBatt(const bool value)
{
    if (m_notif_batt != value)
    {
        m_notif_batt = value;
        writeSettings();
        Q_EMIT notifsChanged();
    }
}

void SettingsManager::setNotifSubzero(const bool value)
{
    if (m_notif_subzero != value)
    {
        m_notif_subzero = value;
        writeSettings();
        Q_EMIT notifsChanged();
    }
}

void SettingsManager::setNotifWater(const bool value)
{
    if (m_notif_water != value)
    {
        m_notif_water = value;
        writeSettings();
        Q_EMIT notifsChanged();
    }
}

void SettingsManager::setNotifEnv(const bool value)
{
    if (m_notif_env != value)
    {
        m_notif_env = value;
        writeSettings();
        Q_EMIT notifsChanged();
    }
}

void SettingsManager::setBluetoothControl(const bool value)
{
    if (m_bluetoothControl != value)
    {
        m_bluetoothControl = value;
        writeSettings();
        Q_EMIT bluetoothControlChanged();
    }
}
void SettingsManager::setBluetoothLimitScanningRange(const bool value)
{
    if (m_bluetoothLimitScanningRange != value)
    {
        m_bluetoothLimitScanningRange = value;
        writeSettings();
        Q_EMIT bluetoothLimitScanningRangeChanged();
    }
}

void SettingsManager::setBluetoothSimUpdates(const unsigned value)
{
    if (m_bluetoothSimUpdates != value)
    {
        m_bluetoothSimUpdates = value;
        writeSettings();
        Q_EMIT bluetoothSimUpdatesChanged();
    }
}

void SettingsManager::setUpdateIntervalBackground(const unsigned value)
{
    if (m_updateIntervalBackground != value)
    {
        m_updateIntervalBackground = value;
        writeSettings();
        Q_EMIT updateIntervalBackgroundChanged();
    }
}

void SettingsManager::setUpdateIntervalPlant(const unsigned value)
{
    if (m_updateIntervalPlant != value)
    {
        m_updateIntervalPlant = value;
        writeSettings();
        Q_EMIT updateIntervalPlantChanged();
    }
}

void SettingsManager::setUpdateIntervalThermo(const unsigned value)
{
    if (m_updateIntervalThermometer!= value)
    {
        m_updateIntervalThermometer = value;
        writeSettings();
        Q_EMIT updateIntervalThermoChanged();
    }
}

void SettingsManager::setUpdateIntervalEnv(const unsigned value)
{
    if (m_updateIntervalEnvironmental!= value)
    {
        m_updateIntervalEnvironmental = value;
        writeSettings();
        Q_EMIT updateIntervalEnvChanged();
    }
}

void SettingsManager::setTempUnit(const QString &value)
{
    if (m_tempUnit != value)
    {
        m_tempUnit = value;
        writeSettings();
        Q_EMIT tempUnitChanged();
    }
}

void SettingsManager::setGraphHistogram(const QString &value)
{
    if (m_graphHistogram != value)
    {
        m_graphHistogram = value;
        writeSettings();
        Q_EMIT graphHistogramChanged();
    }
}

void SettingsManager::setGraphThermometer(const QString &value)
{
    if (m_graphThermometer != value)
    {
        m_graphThermometer = value;
        writeSettings();
        Q_EMIT graphThermometerChanged();
    }
}

void SettingsManager::setGraphAioDays(const int value)
{
    if (m_graphAioDays != value)
    {
        m_graphAioDays = value;
        writeSettings();
        Q_EMIT graphAioChanged();
    }
}

void SettingsManager::setGraphAioShowDots(const bool value)
{
    if (m_graphAioShowDots != value)
    {
        m_graphAioShowDots = value;
        writeSettings();
        Q_EMIT graphAioChanged();
    }
}

void SettingsManager::setBigIndicator(const bool value)
{
    if (m_bigIndicator != value)
    {
        m_bigIndicator = value;
        writeSettings();
        Q_EMIT bigIndicatorChanged();
    }
}

void SettingsManager::setDynaScale(const bool value)
{
    if (m_dynaScale != value)
    {
        m_dynaScale = value;
        writeSettings();
        Q_EMIT dynaScaleChanged();
    }
}

void SettingsManager::setSplitView(const bool value)
{
    if (m_splitView != value)
    {
        m_splitView = value;
        writeSettings();
        Q_EMIT splitViewChanged();
    }
}

void SettingsManager::setOrderBy(const QString &value)
{
    if (m_orderBy != value)
    {
        m_orderBy = value;
        writeSettings();
        Q_EMIT orderByChanged();
    }
}

/* ************************************************************************** */

void SettingsManager::setLocation(const bool value)
{
    if (m_location != value)
    {
        m_location = value;
        writeSettings();
        Q_EMIT locationChanged();
    }
}

void SettingsManager::setSunAndMoon(const bool value)
{
    if (m_sunandmoon != value)
    {
        m_sunandmoon = value;
        writeSettings();
        Q_EMIT sunandmoonChanged();
    }
}

void SettingsManager::setLatitude(const float value)
{
    if (m_location_latitude != value)
    {
        m_location_latitude = value;
        writeSettings();
        Q_EMIT positionChanged();
    }
}

void SettingsManager::setLongitude(const float value)
{
    if (m_location_longitude != value)
    {
        m_location_longitude = value;
        writeSettings();
        Q_EMIT positionChanged();
    }
}

/* ************************************************************************** */

void SettingsManager::setMySQL(const bool value)
{
    if (m_mysql != value)
    {
        m_mysql = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMysqlHost(const QString &value)
{
    if (m_mysqlHost != value)
    {
        m_mysqlHost = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMysqlPort(const int value)
{
    if (m_mysqlPort != value)
    {
        m_mysqlPort = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMysqlUser(const QString &value)
{
    if (m_mysqlUser != value)
    {
        m_mysqlUser = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMysqlPassword(const QString &value)
{
    if (m_mysqlPassword != value)
    {
        m_mysqlPassword = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMQTT(const bool value)
{
    if (m_mqtt != value)
    {
        m_mqtt = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttHost(const QString &value)
{
    if (m_mqttHost != value)
    {
        m_mqttHost = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttPort(const int value)
{
    if (m_mqttPort != value)
    {
        m_mqttPort = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttUser(const QString &value)
{
    if (m_mqttUser != value)
    {
        m_mqttUser = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttPassword(const QString &value)
{
    if (m_mqttPassword != value)
    {
        m_mqttPassword = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttTopics(const QString &value)
{
    if (m_mqttTopics != value)
    {
        m_mqttTopics = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}


/* ************************************************************************** */

QString SettingsManager::getBackendReader() const
{
#if defined(zxingcpp)
    return "zxingcpp";
#elif defined(qzxing)
    return "qzxing";
#endif

    qWarning() << "SettingsManager::getBackendReader() no backend set";
    return "error";
}

QString SettingsManager::getBackendWriter() const
{
    if (m_backendWriter == "qzxing" && getBackendQZXing()) return "qzxing";
    if (m_backendWriter == "zxingcpp" && getBackendZXingCpp()) return "zxingcpp";
    if (m_backendWriter == "zint" && getBackendZint()) return "zint";

    // fallbacks
    if (getBackendZint()) return "zint";
    if (getBackendZXingCpp()) return "zxingcpp";
    if (getBackendQZXing()) return "qzxing";

    qWarning() << "SettingsManager::getBackendWriter() no backend set";
    return "error";
}

void SettingsManager::setBackendWriter(const QString &value)
{
    if (value == "" || value == "qzxing" || value == "zxingcpp" || value == "zint")
    {
        if (m_backendWriter != value)
        {
            m_backendWriter = value;
            Q_EMIT backendWriterChanged();

            writeSettings();
        }
    }
}

bool SettingsManager::getBackendQZXing() const
{
#if defined(qzxing)
    return true;
#endif
    return false;
}

bool SettingsManager::getBackendZXingCpp() const
{
#if defined(zxingcpp)
    return true;
#endif
    return false;
}
bool SettingsManager::getBackendZint() const
{
#if defined(zint)
    return true;
#endif
    return false;
}

void SettingsManager::setDefaultTab(const QString &value)
{
    if (m_defaultTab != value)
    {
        m_defaultTab = value;
        Q_EMIT defaultTabChanged();

        writeSettings();
    }
}

unsigned SettingsManager::getFormatsEnabled() const
{
#if defined(zxingcpp)
    return m_formatsEnabled_zxingcpp;
#elif defined(qzxing)
    return m_formatsEnabled_qzxing;
#endif
    return 0;
}

void SettingsManager::setFormatsEnabled(const unsigned value)
{
#if defined(zxingcpp)
    if (m_formatsEnabled_zxingcpp != value)
    {
        m_formatsEnabled_zxingcpp = value;
        Q_EMIT formatsEnabledChanged();

        writeSettings();
    }
#elif defined(qzxing)
    if (m_formatsEnabled_qzxing != value)
    {
        m_formatsEnabled_qzxing = value;
        Q_EMIT formatsEnabledChanged();

        writeSettings();
    }
#endif
}

void SettingsManager::setSaveBarcode(const bool value)
{
    if (m_save_barcodes != value)
    {
        m_save_barcodes = value;
        Q_EMIT saveChanged();

        writeSettings();
    }
}

void SettingsManager::setSaveCamera(const bool value)
{
    if (m_save_camera != value)
    {
        m_save_camera = value;
        Q_EMIT saveChanged();

        writeSettings();
    }
}

void SettingsManager::setSaveGPS(const bool value)
{
    if (m_save_gps != value)
    {
        m_save_gps = value;
        Q_EMIT saveChanged();

        writeSettings();
    }
}

void SettingsManager::setShowDebug(const bool value)
{
    if (m_showDebug != value)
    {
        m_showDebug = value;
        Q_EMIT debugChanged();

        writeSettings();
    }
}

void SettingsManager::setScanFullresolution(const bool value)
{
    if (m_scan_fullresolution != value)
    {
        m_scan_fullresolution = value;
        Q_EMIT fullresChanged();

        writeSettings();
    }
}

void SettingsManager::setScanFullscreen(const bool value)
{
    if (m_scan_fullscreen != value)
    {
        m_scan_fullscreen = value;
        Q_EMIT fullscreenChanged();

        writeSettings();
    }
}

void SettingsManager::setScanTryHarder(const bool value)
{
    if (m_scan_tryHarder != value)
    {
        m_scan_tryHarder = value;
        Q_EMIT tryHarderChanged();

        writeSettings();
    }
}

void SettingsManager::setScanTryRotate(const bool value)
{
    if (m_scan_tryRotate != value)
    {
        m_scan_tryRotate = value;
        Q_EMIT tryRotateChanged();

        writeSettings();
    }
}

void SettingsManager::setScanTryInvert(const bool value)
{
    if (m_scan_tryInvert != value)
    {
        m_scan_tryInvert = value;
        Q_EMIT tryInvertChanged();

        writeSettings();
    }
}

void SettingsManager::setScanTryDownscale(const bool value)
{
    if (m_scan_tryDownscale != value)
    {
        m_scan_tryDownscale = value;
        Q_EMIT tryDownscaleChanged();

        writeSettings();
    }
}

void SettingsManager::setSysTray(const bool value)
{
    if (m_systrayEnabled != value)
    {
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
        bool trayEnable_saved = m_systrayEnabled;
#endif

        m_systrayEnabled = value;
        writeSettings();

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
            SystrayManager *st = SystrayManager::getInstance();
            if (st)
            {
                if (trayEnable_saved == true && m_systrayEnabled == false)
                {
                    st->removeSystray();
                    Q_EMIT systrayChanged();
                }
                else if (trayEnable_saved == false && m_systrayEnabled == true)
                {
                    st->installSystray();
                    Q_EMIT systrayChanged();
                }
            }
#endif
        }
    }


/* ************************************************************************** */
