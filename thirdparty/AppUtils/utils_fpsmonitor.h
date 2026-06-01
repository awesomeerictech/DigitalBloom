#ifndef UTILS_FPSMONITOR_H
#define UTILS_FPSMONITOR_H
/* ************************************************************************** */

#include <QObject>
#include <QList>
#include <QMutex>
#include <QDateTime>

class QTimer;
class QQuickWindow;

/* ************************************************************************** */

/*!
 * \brief The FrameRateMonitor class
 *
 * This class use the QQuickWindow::frameSwapped method from Luca Carlon.
 * - https://github.com/carlonluca/lqtutils/blob/master/lqtutils_freq.h
 *
 * The FrameMonitor widget uses a simpler and pure QML method from qnanopainter.
 * - https://github.com/QUItCoding/qnanopainter/blob/master/examples/qnanopainter_vs_qpainter_demo/qml/FpsItem.qml
 */
class FrameRateMonitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int fps READ fps NOTIFY fpsChanged)

    QMutex m_mutex;
    QList <QDateTime> m_timestamps;
    QTimer *m_refreshTimer = nullptr;

    int m_fps = 0;
    int fps() const { return m_fps; }

Q_SIGNALS:
    void fpsChanged();

public:
    FrameRateMonitor(QQuickWindow *window = nullptr, QObject *parent = nullptr);
    Q_INVOKABLE void setQuickWindow(QQuickWindow *window);

public Q_SLOTS:
    void registerSample();
    void refresh();
};

/* ************************************************************************** */
#endif // UTILS_FPSMONITOR_H
