#ifndef UTILS_OS_MACOS_DOCK_H
#define UTILS_OS_MACOS_DOCK_H

#include <QtGlobal>

#if defined(Q_OS_MACOS)
/* ************************************************************************** */

#include <QObject>

class QQuickWindow;

/*!
 * \brief macOS dock click handler
 *
 * Use with "LIBS += -framework AppKit"
 */
class MacOSDockHandler : public QObject
{
    Q_OBJECT

    MacOSDockHandler();
    ~MacOSDockHandler();

    QQuickWindow *m_saved_window = nullptr;

signals:
    void dockIconClicked();

public:
    static MacOSDockHandler *getInstance();

    void setupDock(QQuickWindow *window);

    Q_INVOKABLE static void toggleDockIconVisibility(bool show);
};

/* ************************************************************************** */
#endif // Q_OS_MACOS
#endif // UTILS_OS_MACOS_DOCK_H
