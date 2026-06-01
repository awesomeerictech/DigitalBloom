#include "utils_fpsmonitor.h"

#include <QMutableListIterator>
#include <QQuickWindow>
#include <QTimer>
#include <QDebug>

/* ************************************************************************** */

FrameRateMonitor::FrameRateMonitor(QQuickWindow *window, QObject *parent) : QObject(parent)
{
    setQuickWindow(window);

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &FrameRateMonitor::refresh);

    m_refreshTimer->setInterval(1000);
    m_refreshTimer->setSingleShot(true);
}

/* ************************************************************************** */

void FrameRateMonitor::setQuickWindow(QQuickWindow *window)
{
    if (window)
    {
        connect(window, &QQuickWindow::frameSwapped, this, &FrameRateMonitor::registerSample);
    }
    else
    {
        qWarning() << "FrameRateMonitor::setWindow() No QQuickWindow available";
    }
}

void FrameRateMonitor::registerSample()
{
    QMutexLocker locker(&m_mutex);
    m_timestamps.append(QDateTime::currentDateTime());
    QTimer::singleShot(0, this, &FrameRateMonitor::refresh);
}

void FrameRateMonitor::refresh()
{
    QMutexLocker locker(&m_mutex);
    QDateTime now = QDateTime::currentDateTime();

    QMutableListIterator <QDateTime> it(m_timestamps);
    while (it.hasNext()) {
        if (it.next().msecsTo(now) > 1000) {
            it.remove();
        } else {
            break;
        }
    }

    m_fps = m_timestamps.size();
    Q_EMIT fpsChanged();

    m_refreshTimer->start();
}

/* ************************************************************************** */
