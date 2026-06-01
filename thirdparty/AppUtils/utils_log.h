#ifndef UTILS_LOG_H
#define UTILS_LOG_H
/* ************************************************************************** */

#include <QObject>
#include <QString>
#include <QFile>

/* ************************************************************************** */

class UtilsLog : public QObject
{
    Q_OBJECT

    bool m_logging = false;
    QString m_logPath;
    QFile m_logFile;

    // Singleton
    static UtilsLog *instance;
    UtilsLog(const bool enabled);
    UtilsLog();
    ~UtilsLog();

public:
    static UtilsLog *getInstance(const bool enabled = true);

    void setEnabled(const bool enabled);

    bool openLogFile(const QString &path = QString());

    Q_INVOKABLE void pushLog(const QString &log);

    Q_INVOKABLE QString getLog();

    Q_INVOKABLE void clearLog();
};

/* ************************************************************************** */
#endif // UTILS_LOG_H
