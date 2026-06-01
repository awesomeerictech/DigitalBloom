#ifndef MYTIMER_HPP
#define MYTIMER_HPP

#include <QObject>
#include <QElapsedTimer>

class MyTimer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int elapsed MEMBER m_elapsed NOTIFY elapsedChanged)
    Q_PROPERTY(bool running MEMBER m_running NOTIFY runningChanged)
private:
    QElapsedTimer m_timer;
    int m_elapsed;
    bool m_running;
public Q_SLOTS:
    void start() {
        this->m_elapsed = 0;
        this->m_running = true;

        m_timer.start();
        Q_EMIT runningChanged();
    }

    void stop() {
        this->m_elapsed = m_timer.elapsed();
        this->m_running = false;

        Q_EMIT elapsedChanged();
        Q_EMIT runningChanged();
    }

Q_SIGNALS:
    void runningChanged();
    void elapsedChanged();
};

#endif // MYTIMER_HPP
