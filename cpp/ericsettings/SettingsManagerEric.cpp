#include "SettingsManagerEric.h"
// Qt
#include <QLocale>
#include <QStandardPaths>
#include <QList>
#include <QDir>
// qutils
#include "../Macros.h"

#define COL_SETTING_NAME "setting_name"
#define COL_SETTING_VALUE "setting_value"
#define COL_SETTING_TYPE "setting_type"
#define DATABASE_CHECK() do { if (m_Database.isOpen() == false) { openDatabase(); createTable(); } } while (0)


QList<SettingsManagerEric *> SettingsManagerEric::m_Instances = QList<SettingsManagerEric *>();
int SettingsManagerEric::m_InstanceLastIndex = 0;

SettingsManagerEric::SettingsManagerEric(QString databaseName, QString tableName, QObject *parent)
    : QObject(parent)
    , m_InstanceIndex(m_InstanceLastIndex)
    , m_DatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/dberic/" + databaseName)
    , m_SettingsTableName(tableName)
    , m_SqlManager()
    , m_Database()
{
    m_Instances.append(this);
    m_InstanceLastIndex++;

    // Create the app data location folder if it doesn't exist.
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QDir dbdir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/dberic");
    if (dir.exists() == false) {
        dir.mkpath(dir.path());

    }
    if (dbdir.exists() == false) {
        dbdir.mkpath(dbdir.path());

    }
}

SettingsManagerEric::~SettingsManagerEric()
{
    m_Instances[m_InstanceIndex] = nullptr;
}

QString SettingsManagerEric::getSystemLanguage() const
{
    QLocale locale;
    QString name = locale.name();
    return name.left(name.indexOf("_"));
}

bool SettingsManagerEric::write(const QString &key, const QVariant &value)
{
    DATABASE_CHECK();

    bool successful = false;
    const QList<SqliteManager::Constraint> values {
        std::make_tuple(COL_SETTING_NAME, key, "AND")
    };

    const QList<QMap<QString, QVariant>> existingData = m_SqlManager.getFromTable(m_Database, m_SettingsTableName, -1, &values);
    const bool exists = existingData.size() > 0;
    QMap<QString, QVariant> newMap;
    newMap[COL_SETTING_NAME] = key;
    newMap[COL_SETTING_VALUE] = value.toByteArray();
    newMap[COL_SETTING_TYPE] = QVariant::fromValue<int>(value.type());

    if (exists) {
        const QVariantMap oldMap = existingData.at(0);
        successful = m_SqlManager.updateInTable(m_Database, m_SettingsTableName, newMap, values);
        QVariant oldValue = oldMap[COL_SETTING_VALUE];
        oldValue.convert(oldMap[COL_SETTING_VALUE].toInt());

        emitSettingChangedInAllInstances(key, oldValue, value);
    }
    else {
        successful = m_SqlManager.insertIntoTable(m_Database, m_SettingsTableName, newMap);
        emitSettingChangedInAllInstances(key, "", value);
    }

    return successful;
}

QVariant SettingsManagerEric::read(const QString &key)
{
    DATABASE_CHECK();

    const QList<SqliteManager::Constraint> values {
        std::make_tuple(COL_SETTING_NAME, key, "AND")
    };

    QVariant value;
    const QList<QMap<QString, QVariant>> existingData = m_SqlManager.getFromTable(m_Database, m_SettingsTableName, -1, &values);
    const bool exists = existingData.size() > 0;
    if (exists) {
        value = existingData.at(0)[COL_SETTING_VALUE];
        value.convert(existingData.at(0)[COL_SETTING_TYPE].toInt());
    }

    return value;
}

bool SettingsManagerEric::remove(const QString &key)
{
    DATABASE_CHECK();

    const QList<SqliteManager::Constraint> constraints {
        std::make_tuple(COL_SETTING_NAME, key, "AND")
    };

    return m_SqlManager.deleteInTable(m_Database, m_SettingsTableName, constraints);
}

bool SettingsManagerEric::exists(const QString &key)
{
    DATABASE_CHECK();

    const QList<SqliteManager::Constraint> constraints {
        std::make_tuple(COL_SETTING_NAME, key, "AND")
    };

    return m_SqlManager.exists(m_Database, m_SettingsTableName, constraints);
}

QString SettingsManagerEric::getDatabaseName() const
{
    return m_DatabaseName;
}

void SettingsManagerEric::setDatabaseName(const QString &databaseName)
{
    if (QDir::isAbsolutePath(databaseName)) {
        LOG_ERROR("Absolute path is given! Database name is just the file name.");
        return;
    }

    m_DatabaseName = databaseName;
    Q_EMIT databasePathChanged();

    restartDatabase();
    createTable();
}

QString SettingsManagerEric::getSettingsTableName() const
{
    return m_SettingsTableName;
}

void SettingsManagerEric::setSettingsTableName(const QString &tableName)
{
    m_SettingsTableName = tableName;
    Q_EMIT settingsTableNameChanged();

    restartDatabase();
    createTable();
}

void SettingsManagerEric::createTable()
{
    DATABASE_CHECK();

    QList<SqliteManager::ColumnDefinition> columns {
        SqliteManager::ColumnDefinition(false, SqliteManager::ColumnTypes::TEXT, COL_SETTING_NAME),
        SqliteManager::ColumnDefinition(false, SqliteManager::ColumnTypes::BLOB, COL_SETTING_VALUE),
        SqliteManager::ColumnDefinition(false, SqliteManager::ColumnTypes::INTEGER, COL_SETTING_TYPE)
    };

    m_SqlManager.createTable(m_Database, columns, m_SettingsTableName);
}

void SettingsManagerEric::openDatabase()
{
    if (m_Database.isOpen() == false) {
        m_Database = m_SqlManager.openDatabase(m_DatabaseName);
        Q_EMIT databaseOpened();
    }
}

void SettingsManagerEric::restartDatabase()
{
    if (m_Database.isOpen()) {
        m_SqlManager.closeDatabase(m_Database);
        Q_EMIT databaseClosed();
    }

    m_Database = m_SqlManager.openDatabase(m_DatabaseName);
    Q_EMIT databaseOpened();
}

void SettingsManagerEric::emitSettingChangedInAllInstances(const QString &settingName, const QVariant &oldSettingValue, const QVariant &newSettingValue)
{
    if (oldSettingValue != newSettingValue) {
        for (SettingsManagerEric *man : m_Instances) {
            if (man) {
                man->emitSettingChanged(settingName, oldSettingValue, newSettingValue);
            }
        }
    }
}

void SettingsManagerEric::emitSettingChanged(const QString &settingName, const QVariant &oldSettingValue, const QVariant &newSettingValue)
{
    Q_EMIT settingChanged(settingName, oldSettingValue, newSettingValue);
}


