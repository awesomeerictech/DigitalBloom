#ifndef RESTCONSUMER_H
#define RESTCONSUMER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlListProperty>
#include <QStringListModel>
#include <QHttpPart>
#include <QFile>
#include <QFileInfo>
#include <QSslCertificate>
#include <QSsl>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QSslKey>
#include <QSslCertificateExtension>
#include <QSslCipher>
#include <QSslDiffieHellmanParameters>
#include <QSslEllipticCurve>
#include <QSslError>
#include <QSslPreSharedKeyAuthenticator>

#include "mimetypes.h"

namespace eric {


class RestConsumer : public QObject
{
    Q_PROPERTY(QByteArray host READ host WRITE setHost NOTIFY hostChanged)

    Q_OBJECT

public:
    RestConsumer();
    ~RestConsumer();

public Q_SLOTS:

    QByteArray host();
    void setHost(QByteArray host);

    void addHeader(QByteArray key, QByteArray value);

    void get(QByteArray resource,  QString query = "");
    void post(QByteArray resource, QByteArray data, QString query = "");

    void put(QByteArray resource, QByteArray data, QString query = "");
    void remove(QByteArray resource, QString query = "");

    void upload(QByteArray resource, QUrl file, bool put);
    void upload(QByteArray resource, QUrl file, QByteArray data, bool put);

Q_SIGNALS:

    void ready(const QByteArray rawData);
    void posted(const QByteArray rawData);
    void updated(const QByteArray rawData);
    void deleted(const QByteArray rawData);

    void uploaded(const QByteArray rawData);

    void networkError( QNetworkReply::NetworkError err );
    void serverError(const QByteArray err);

    // errors issued by RestConsumer before sending a request
    void error(const QByteArray err);

    void hostChanged(QByteArray host);

private Q_SLOTS:
    void parseNetworkResponse(QNetworkReply *reply );

private:
    void setQueryParams(QUrl &url, QString params);
    void addHeaders(QByteArray headers);
    void setHeaders(QNetworkRequest &request);

    MimeTypes mimeTypes;

    QHash<QByteArray, QByteArray> headers;
    QByteArray m_host;
    QNetworkAccessManager networkAccessManager;
};

}
#endif // RESTCONSUMER_H
