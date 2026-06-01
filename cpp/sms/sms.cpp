#include "sms.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QDebug>
#include <QUrlQuery>

//  https://api.sandbox.africastalking.com/version1/messaging

//  https://api.africastalking.com/version1/messaging


const static QUrl server(QLatin1String("https://api.africastalking.com"));
const static QString usersApiPath(QLatin1String("/version1/messaging"));

Sms::Sms(QObject *parent)
    : QObject(parent)
{
    connect(&_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
    connect(&_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
}

void Sms::singleMessage(QByteArray username, QByteArray to, QByteArray message,QByteArray from,QByteArray bulkSMSMode)
{

    QUrl url = server;
    url.setPath(usersApiPath);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("username", username);
    urlQuery.addQueryItem("to", to);
    urlQuery.addQueryItem("message", message);
    urlQuery.addQueryItem("from",from);
    urlQuery.addQueryItem("bulkSMSMode",bulkSMSMode);
    url.setQuery(urlQuery);

    QNetworkRequest request(url);
    request.setRawHeader("apiKey", "a36af9292283d320e758aef77d993bea616f8b7d47614eb8537cb382d17d6a0e");
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader,
        "application/x-www-form-urlencoded");
     _manager.post(request, urlQuery.toString(QUrl::FullyEncoded).toUtf8());

    qDebug() << "The encoded Url is: " << urlQuery.toString(QUrl::FullyEncoded).toUtf8() << Qt::endl;



}

void Sms::finished(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();
    qDebug() << "Reply: " << data;

}

void Sms::sslErrors(QNetworkReply *reply, QList<QSslError> list)
{
    QByteArray data = reply->readAll();
    qDebug() << __FILE__ << __LINE__ << list;
}
