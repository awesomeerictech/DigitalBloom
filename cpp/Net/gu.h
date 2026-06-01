#ifndef GU_H
#define GU_H

#include <QtQml>
#include "include.h"

namespace eric {


class Gu : public QObject
{
    Q_OBJECT

public:
    Gu(){}

    static void registerTypes(){

        qmlRegisterType<eric::Rest>("Org.Eric.Technologies", 1, 0, "Rest");
        qmlRegisterType<eric::RestUploader>("Org.Eric.Technologies", 1, 0, "RestUploader");
        qmlRegisterType<eric::RestConsumer>("Org.Eric.Technologies", 1, 0, "RestConsumer");
        qmlRegisterType<eric::RestConsumer2>("Org.Eric.Technologies", 1, 0, "RestConsumer2");
        qmlRegisterType<eric::RestConsumer3>("Org.Eric.Technologies", 1, 0, "RestConsumer3");
        qmlRegisterType<eric::RestConsumer3>("Org.Eric.Technologies", 1, 0, "RestConsumer3");
        qmlRegisterType<eric::FileIO>("Org.Eric.Technologies", 1, 0, "FileIO");
        qmlRegisterType<eric::RestModel>("Org.Eric.Technologies", 1, 0, "RestModel");
        qmlRegisterType<QStringListModel>("Org.Eric.Technologies", 1, 0, "QStringListModel");
    }

public Q_SLOTS:

    static QString timeAgo(QString timestamp){
        return timeAgo(QDateTime::fromString(timestamp, "dd/MM/yyyy hh:mm:ss").toMSecsSinceEpoch());
    }
    static QString timeAgo(QDateTime timestamp){
        return timeAgo(timestamp.toMSecsSinceEpoch());
    }

    
    static QString timeAgo(qint64 timestamp)
    {
        qint64 currentTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

        bool isFuture = false;
        qint64 secDiff = 0;

        if ( timestamp > currentTimestamp )//future
        {
            isFuture = true;
            secDiff = (timestamp - currentTimestamp) / 1000;
        }
        else if ( timestamp < currentTimestamp )//past
        {
            isFuture = false;
            secDiff = (currentTimestamp - timestamp) / 1000;
        }
        else//the same
        {
            return QString(tr("Just now"));
        }

        QString ret;

        if ( secDiff > 365*24*60*60 )//longer than 1 year
        {
            int years = secDiff / (365*24*60*60);
            if ( isFuture )
            {
                ret = tr("%1 years in the future").arg(years);
            }
            else
            {
                ret = tr("%1 years ago").arg(years);
            }
        }
        else if ( secDiff <= 365*24*60*60 && 30*24*60*60 < secDiff )//shorter than 1 year and longer than 1 month
        {
            int months = secDiff / (30*24*60*60);
            if ( isFuture )
            {
                ret = tr("%1 months in the future").arg(months);
            }
            else
            {
                ret = tr("%1 months ago").arg(months);
            }
        }
        else if ( secDiff <= 30*24*60*60 && 24*60*60 < secDiff )//shorter than 1 month and longer than 1 day
        {
            int days = secDiff / (24*60*60);
            if ( isFuture )
            {
                ret = tr("%1 days in the future").arg(days);
            }
            else
            {
                ret = tr("%1 days ago").arg(days);
            }
        }
        else if ( secDiff <= 24*60*60 && 60*60 < secDiff )//shorter than 1 day and longer than 1 hour
        {
            int hours = secDiff / (60*60);
            if ( isFuture )
            {
                ret = tr("%1 hours in the future").arg(hours);
            }
            else
            {
                ret = tr("%1 hours ago").arg(hours);
            }
        }
        else if ( secDiff <= 60*60 && 60 < secDiff )//shorter than 1 hour and longer than 1 minute
        {
            int minutes = secDiff / (60);
            if ( isFuture )
            {
                ret = tr("%1 minutes in the future").arg(minutes);
            }
            else
            {
                ret = tr("%1 minutes ago").arg(minutes);
            }
        }
        else if ( secDiff <= 60 )//shorter than 1 minute
        {
            if ( isFuture )
            {
                ret = tr("less than 1 minute in the future");
            }
            else
            {
                ret = tr("less than 1 minute ago");
            }
        }
        return ret;
    }
};

}

#endif // GU_H
