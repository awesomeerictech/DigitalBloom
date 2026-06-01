#ifndef UTILS_LANGUAGE_H
#define UTILS_LANGUAGE_H
/* ************************************************************************** */

#include <QObject>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QTranslator>
#include <QString>

/* ************************************************************************** */

class UtilsLanguage : public QObject
{
    Q_OBJECT

    QString m_appName;
    QString m_appLanguage;

    QString m_locale_str_full;
    QString m_locale_str_short;

    QCoreApplication *m_qt_app = nullptr;
    QQmlApplicationEngine *m_qml_engine = nullptr;

    QTranslator *m_qtTranslator = nullptr;
    QTranslator *m_appTranslator = nullptr;

    // Singleton
    static UtilsLanguage *instance;
    UtilsLanguage();
    ~UtilsLanguage();

public:
    static UtilsLanguage *getInstance();

    void setAppName(const QString &name, const bool forceLowerCase = false);
    void setAppInstance(QCoreApplication *app);
    void setQmlEngine(QQmlApplicationEngine *engine);

    Q_INVOKABLE void loadLanguage(const QString &lng);

    Q_INVOKABLE QString getCurrentLanguage() const { return m_appLanguage; }
    Q_INVOKABLE QString getCurrentLanguageCode_full() const { return m_locale_str_full; }
    Q_INVOKABLE QString getCurrentLanguageCode_short() const { return m_locale_str_short; }
};

/* ************************************************************************** */
#endif // UTILS_LANGUAGE_H
