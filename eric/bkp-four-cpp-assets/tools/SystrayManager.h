#ifndef SYSTRAY_MANAGER_H
#define SYSTRAY_MANAGER_H
/* ************************************************************************** */

#include <QtGlobal>
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>

class QMenu;
class QAction;
class QApplication;
class QQuickWindow;

/* ************************************************************************** */

/*!
 * \brief The SystrayManager class
 */
class SystrayManager: public QObject
{
    Q_OBJECT

    QApplication *m_saved_app = nullptr;
    QQuickWindow *m_saved_window = nullptr;

    QSystemTrayIcon *m_sysTray = nullptr;
    QIcon *m_sysTrayIcon = nullptr;
    QMenu *m_sysTrayMenu = nullptr;
    QAction *m_actionShow = nullptr;
    QAction *m_actionDeviceList = nullptr;
    QAction *m_actionSettings = nullptr;
    QAction *m_actionExit = nullptr;

    QTimer m_retryTimer;
    int retryCount = 6;

    static SystrayManager *instance;

    SystrayManager();
    ~SystrayManager();

    void initSystray();

Q_SIGNALS:
    void showClicked();
    void hideClicked();
    void sensorsClicked();
    void settingsClicked();
    void quitClicked();

public:
    static SystrayManager *getInstance();
    void setupSystray(QQuickWindow *window);

public Q_SLOTS:
    bool installSystray();
    void REinstallSystray();
    void removeSystray();
    void sendNotification(const QString &text);

private Q_SLOTS:
    void trayClicked(const QSystemTrayIcon::ActivationReason r);
    void showHideButton();
    void sensorsButton();
    void settingsButton();

    void visibilityChanged();
    void aboutToBeDestroyed();
};

/* ************************************************************************** */
#endif // !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
#endif // SYSTRAY_MANAGER_H
