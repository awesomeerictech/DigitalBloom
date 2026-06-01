#include "SystrayManager.h"

#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QQuickWindow>
#include <QSystemTrayIcon>

/* ************************************************************************** */

SystrayManager *SystrayManager::instance = nullptr;

SystrayManager *SystrayManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new SystrayManager();
    }

    return instance;
}

SystrayManager::SystrayManager()
{
    // Connect retry timer
    connect(&m_retryTimer, &QTimer::timeout, this, &SystrayManager::installSystray);
}

SystrayManager::~SystrayManager()
{
    delete m_actionShow;
    delete m_actionDeviceList;
    delete m_actionSettings;
    delete m_actionExit;
    m_actionShow = nullptr;
    m_actionSettings = nullptr;
    m_actionExit = nullptr;

    delete m_sysTrayIcon;
    delete m_sysTrayMenu;
    m_sysTrayIcon = nullptr;
    m_sysTrayMenu = nullptr;

    removeSystray();
}

/* ************************************************************************** */

void SystrayManager::setupSystray(QQuickWindow *window)
{
    if (!qApp || !window)
    {
        qWarning() << "SystrayManager::setupSystray() no QApplication or QQuickWindow passed";
        return;
    }

    m_saved_app = qApp;
    m_saved_window = window;
}

/* ************************************************************************** */

void SystrayManager::initSystray()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    return;
#endif

    if (!m_saved_app || !m_saved_window)
    {
        qWarning() << "SystrayManager::initSystray() no QApplication or QQuickWindow saved";
        return;
    }

    if (m_sysTrayMenu == nullptr)
    {
        m_sysTrayMenu = new QMenu();
        if (m_sysTrayMenu)
        {
            m_actionDeviceList = new QAction(tr("Sensor list"));
            m_actionSettings = new QAction(tr("Settings") + "        ");
            m_actionShow = new QAction(tr("Hide"));
            if (!m_saved_window ||
                m_saved_window->isVisible() == false ||
                m_saved_window->visibility() == QWindow::Hidden ||
                m_saved_window->visibility() == QWindow::Minimized)
            {
                m_actionShow->setText(tr("Show"));
            }
            m_actionExit = new QAction(tr("Quit"));

            m_sysTrayMenu->addAction(m_actionDeviceList);
            m_sysTrayMenu->addAction(m_actionSettings);
            m_sysTrayMenu->addSeparator();
            m_sysTrayMenu->addAction(m_actionShow);
            m_sysTrayMenu->addAction(m_actionExit);

            connect(m_actionShow, &QAction::triggered, this, &SystrayManager::showHideButton);
            connect(m_actionDeviceList, &QAction::triggered, this, &SystrayManager::sensorsButton);
            connect(m_actionSettings, &QAction::triggered, this, &SystrayManager::settingsButton);
            connect(m_actionExit, &QAction::triggered, m_saved_app, &QApplication::exit);
        }

#if defined(Q_OS_MACOS)
        m_sysTrayIcon = new QIcon(":/assets/gfx/logos/digitalbloom_tray_dark.svg");
#else
        m_sysTrayIcon = new QIcon(":/assets/gfx/logos/digitalbloom_tray_dark.svg");
#endif
    }
}

bool SystrayManager::installSystray()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    return false;
#endif

    bool status = false;

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        if (m_sysTray == nullptr)
        {
            initSystray();
            m_sysTray = new QSystemTrayIcon();
        }

        if (m_sysTray != nullptr && m_sysTrayMenu != nullptr && m_sysTrayIcon != nullptr)
        {
#if !defined(Q_OS_MACOS)
            connect(m_sysTray, &QSystemTrayIcon::activated, this, &SystrayManager::trayClicked);
#endif
            m_sysTray->setIcon(*m_sysTrayIcon);
            m_sysTray->setContextMenu(m_sysTrayMenu);
            m_sysTray->show();

            connect(m_sysTray, &QSystemTrayIcon::destroyed, this, &SystrayManager::aboutToBeDestroyed);
            connect(m_saved_window, &QQuickWindow::visibilityChanged, this, &SystrayManager::visibilityChanged);

            // Show greetings
            //m_sysTray->showMessage("digitalbloom", tr("digitalbloom is running in the background!"));

            status = true;
        }
        else
        {
            qWarning() << "SystrayManager::installSystray() Cannot install systray...";
        }
    }
    else
    {
        if (retryCount > 0)
        {
            m_retryTimer.setSingleShot(true);
            m_retryTimer.start(3333);
            retryCount--;
        }
        else
        {
           qWarning() << "SystrayManager::installSystray() Empty systray counter";
        }
    }

    return status;
}

void SystrayManager::REinstallSystray()
{
    // Trying to launch a new instance will manually hide then show again the tray icon...
    // This hack helps in cases where the tray icon just disappears after some time, seen a lot on Gnome desktop with TopIcons Plus.
    // A different solution to this problem is to use gnome-shell-extension-appindicator instead of TopIcons Plus.

#if defined(Q_OS_LINUX)
    if (m_sysTray)
    {
        if (m_sysTrayIcon && m_sysTrayMenu)
        {
            m_sysTray->hide();
            m_sysTray->show();
        }
        else
        {
            qWarning() << "SystrayManager::REinstallSystray() ERROR";
        }
    }
#endif // Q_OS_LINUX
}

void SystrayManager::removeSystray()
{
    if (m_sysTray)
    {
        m_retryTimer.stop();
        disconnect(m_saved_window, &QQuickWindow::visibilityChanged, this, &SystrayManager::visibilityChanged);
        disconnect(m_sysTray, &QSystemTrayIcon::activated, this, &SystrayManager::trayClicked);
        disconnect(m_sysTray, &QSystemTrayIcon::destroyed, this, &SystrayManager::aboutToBeDestroyed);

        retryCount = 6;

        delete m_sysTray;
        m_sysTray = nullptr;
    }
}

/* ************************************************************************** */

void SystrayManager::sendNotification(const QString &text)
{
    if (m_sysTray && QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_sysTray->showMessage("digitalbloom", text);
    }
}

/* ************************************************************************** */

void SystrayManager::trayClicked(const QSystemTrayIcon::ActivationReason r)
{
    // Context, DoubleClick, Trigger, MiddleClick

    if (r == QSystemTrayIcon::Context)
    {
        // do nothing
    }
    else
    {
        showHideButton();
    }
}

void SystrayManager::showHideButton()
{
    if (m_saved_window->isVisible())
    {
        m_saved_window->hide();
    }
    else
    {
        m_saved_window->show();
        m_saved_window->raise();
    }
}

void SystrayManager::sensorsButton()
{
    m_saved_window->show();
    m_saved_window->raise();
    Q_EMIT sensorsClicked();
}

void SystrayManager::settingsButton()
{
    m_saved_window->show();
    m_saved_window->raise();
    Q_EMIT settingsClicked();
}

/* ************************************************************************** */

void SystrayManager::visibilityChanged()
{
    if (m_saved_window->isVisible())
    {
        m_actionShow->setText(tr("Hide"));
    }
    else
    {
        m_actionShow->setText(tr("Show"));
    }
}

void SystrayManager::aboutToBeDestroyed()
{
    qDebug() << "aboutToBeDestroyed()";
}

/* ************************************************************************** */
