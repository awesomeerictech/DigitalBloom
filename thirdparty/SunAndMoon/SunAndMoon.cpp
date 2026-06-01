#include "SunAndMoon.h"

/* ************************************************************************** */

SunAndMoon::SunAndMoon(QObject *parent) : QObject(parent)
{
    //
}

SunAndMoon::~SunAndMoon()
{
    //
}

/* ************************************************************************** */

bool SunAndMoon::isValid() const
{
    if (m_latitude_saved > -90.0 && m_latitude_saved < 90.0 &&
        m_longitude_saved > -180.0 && m_longitude_saved < 180.0)
    {
        if (m_latitude_saved != 0.0 || m_longitude_saved != 0.0)
        {
            return m_lastupdate.isValid();
        }
    }

    qWarning() << "SunAndMoon::isValid() ERROR invalid latitude or longitude"
               << m_latitude_saved << m_longitude_saved;

    return false;
}

void SunAndMoon::update()
{
    if (m_latitude_saved > -720.0 && m_longitude_saved > -720.0)
    {
        if (m_latitude_saved != 0.0 || m_longitude_saved != 0.0)
        {
            set(m_latitude_saved, m_longitude_saved, QDateTime::currentDateTimeUtc());
        }
    }
    else
    {
        qWarning() << "SunAndMoon::update() ERROR latitude and longitude not set";
    }
}

void SunAndMoon::set(const double latitude, const double longitude, const QDateTime datetime)
{
    //qDebug() << "SunAndMoon::set(" << latitude << "," << longitude << "/" << datetime << ")";

    //QDateTime t1 = QDateTime::currentDateTime(); // benchmark

    // Latitude and longitude must be valid
    if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0  || longitude > 180.0)
    {
        qWarning() << "SunAndMoon::set() ERROR invalid latitude or longitude"
                   << latitude << longitude;
        return;
    }

    // Results are cached for 10 minutes
    if (m_latitude_saved == latitude && m_longitude_saved == longitude &&
        m_lastupdate.isValid() && m_lastupdate.secsTo(datetime) < 600) return;

    m_latitude_saved = latitude;
    m_longitude_saved = longitude;
    m_lastupdate = datetime;
    time_t time = datetime.toSecsSinceEpoch();

    m_sr.calculate(latitude, longitude, time);
    m_mr.calculate(latitude, longitude, time);
    m_mp.calculate(time);

    int interval_s1 = (m_sr.setTime - m_sr.riseTime);
    if (interval_s1 < 0) interval_s1 += 24*3600;
    int interval_s2 = (QDateTime::currentSecsSinceEpoch() - m_sr.riseTime);
    if (interval_s2 < 0) interval_s2 += 24*3600;
    sun_percent = std::round((interval_s2 / static_cast<float>(interval_s1)) * 100.f);
    //if (sun_percent < 0) sun_percent = 0;
    //if (sun_percent > 100) sun_percent = 100;

    int interval_m1 = (m_mr.setTime - m_mr.riseTime);
    if (interval_m1 < 0) interval_m1 += 24*3600;
    int interval_m2 = (QDateTime::currentSecsSinceEpoch() - m_mr.riseTime);
    if (interval_m2 < 0) interval_m2 += 24*3600;
    moon_percent = std::round((interval_m2 / static_cast<float>(interval_m1)) * 100.f);
    //if (moon_percent < 0) moon_percent = 0;
    //if (moon_percent > 100) moon_percent = 100;

    Q_EMIT updated();

    //QDateTime t2 = QDateTime::currentDateTime(); // benchmark
    //int64_t load_ms = t2.toMSecsSinceEpoch() - t1.toMSecsSinceEpoch();
    //qDebug() << "SunAndMoon::update()" << load_ms << "ms";
}

void SunAndMoon::print()
{
    qDebug() << "SunAndMoon::print()";
    qDebug() << "- latitude: " << m_latitude_saved << " / longitude: " << m_longitude_saved;
    qDebug() << "- SunRise: " << QDateTime::fromSecsSinceEpoch(m_sr.riseTime, Qt::UTC, 0);
    qDebug() << "- SunSet: " << QDateTime::fromSecsSinceEpoch(m_sr.setTime, Qt::UTC, 0);
    qDebug() << "- MoonRise: " << QDateTime::fromSecsSinceEpoch(m_mr.riseTime, Qt::UTC, 0);
    qDebug() << "- MoonSet: " << QDateTime::fromSecsSinceEpoch(m_mr.setTime, Qt::UTC, 0);
}

/* ************************************************************************** */

bool SunAndMoon::isItDay() const
{
    if (isValid())
    {
        return (sun_percent >= 0 && sun_percent <= 100);
    }
    else
    {
        if (QDateTime::currentDateTime().time().hour() > 8 &&
            QDateTime::currentDateTime().time().hour() < 21)
        {
            return false;
        }
    }

    return true;
}

/* ************************************************************************** */
