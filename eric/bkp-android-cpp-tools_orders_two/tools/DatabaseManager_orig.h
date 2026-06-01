#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H
/* ************************************************************************** */

#include <QObject>
#include <QString>

/* ************************************************************************** */

/*!
 * \brief The DatabaseManager class
 */
class DatabaseManager: public QObject
{
    Q_OBJECT

    const static int s_dbCurrentVersion = 3;

    bool m_dbInternalAvailable = false;
    bool m_dbInternalOpen = false;
    bool m_dbExternalAvailable = false;
    bool m_dbExternalOpen = false;

    bool openDatabase_sqlite();
    bool openDatabase_mysql();
    void closeDatabase();

    void createDatabase();
    void deleteDatabase();

    bool tableExists(const QString &tableName);
    void migrateDatabase();
    bool migrate_v1v2();
    bool migrate_v2v3();

    // Singleton
    static DatabaseManager *instance;
    DatabaseManager();
    ~DatabaseManager();

public:
    static DatabaseManager *getInstance();

    Q_INVOKABLE bool hasDatabaseInternal() const { return m_dbInternalOpen; }
    Q_INVOKABLE bool hasDatabaseExternal() const { return m_dbExternalOpen; }

    Q_INVOKABLE QString getDatabaseDirectory();
    Q_INVOKABLE void resetDatabase();
    Q_INVOKABLE bool saveDatabase();
    Q_INVOKABLE bool restoreDatabase();
};

/* ************************************************************************** */
#endif // DATABASE_MANAGER_H
