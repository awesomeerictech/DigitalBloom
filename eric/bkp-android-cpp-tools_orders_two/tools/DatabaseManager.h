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

    const static int s_dbCurrentVersion = 0;

    bool m_dbInternalAvailable = false;
    bool m_dbInternalOpen = false;
    bool m_dbExternalAvailable = false;
    bool m_dbExternalOpen = false;

    bool openDatabase_sqlite();
    void closeDatabase();

    void createDatabase();
    void deleteDatabase();

    bool tableExists(const QString &tableName);
    void migrateDatabase();

    // Singleton
    static DatabaseManager *instance;
    DatabaseManager();
    ~DatabaseManager();

public:
    static DatabaseManager *getInstance();

    Q_INVOKABLE bool hasDatabaseInternal() const { return m_dbInternalOpen; }
    Q_INVOKABLE bool hasDatabaseExternal() const { return m_dbExternalOpen; }

    Q_INVOKABLE void resetDatabase();
};

/* ************************************************************************** */
#endif // DATABASE_MANAGER_H
