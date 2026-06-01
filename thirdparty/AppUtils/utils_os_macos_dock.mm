#include "utils_os_macos_dock.h"

#if defined(Q_OS_MACOS)

#include <QQuickWindow>
#include <objc/runtime.h>
#include <AppKit/AppKit.h>

#import <Foundation/Foundation.h>

static MacOSDockHandler *instance = nullptr;

/* ************************************************************************** */

bool dockClickHandler(id self, SEL _cmd, ...)
{
    Q_UNUSED(self)
    Q_UNUSED(_cmd)

    if (instance)
    {
        Q_EMIT instance->dockIconClicked();
    }

    // Return NO (false) to suppress the default macOS actions
    return false;
}

/* ************************************************************************** */

MacOSDockHandler *MacOSDockHandler::getInstance()
{
    if (instance == nullptr)
    {
        instance = new MacOSDockHandler();
    }

    return instance;
}

MacOSDockHandler::MacOSDockHandler() : QObject()
{
    // Setup dock click handler
    Class delClass = (Class)[[[NSApplication sharedApplication] delegate] class];
    SEL shouldHandle = sel_registerName("applicationShouldHandleReopen:hasVisibleWindows:");
    class_replaceMethod(delClass, shouldHandle, reinterpret_cast<IMP>(dockClickHandler), "B@:");
}

MacOSDockHandler::~MacOSDockHandler()
{
    delete instance;
}

/* ************************************************************************** */

void MacOSDockHandler::setupDock(QQuickWindow *window)
{
    if (!window)
    {
        qWarning() << "MacOSDockHandler::setupDock() no QQuickWindow passed";
        return;
    }

    m_saved_window = window;

    QObject::connect(this, &MacOSDockHandler::dockIconClicked, m_saved_window, &QQuickWindow::show);
    QObject::connect(this, &MacOSDockHandler::dockIconClicked, m_saved_window, &QQuickWindow::raise);
}

void MacOSDockHandler::toggleDockIconVisibility(bool show)
{
    ProcessSerialNumber psn = {0, kCurrentProcess};

    if (show)
    {
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    }
    else
    {
        TransformProcessType(&psn, kProcessTransformToUIElementApplication);
    }
}

/* ************************************************************************** */
#endif // Q_OS_MACOS
