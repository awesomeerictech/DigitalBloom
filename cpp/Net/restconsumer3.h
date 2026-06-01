#ifndef RESTCONSUMER3_H
#define RESTCONSUMER3_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
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

class RestUploader;

class RestConsumer3 : public QObject
{
    Q_PROPERTY(QByteArray host READ host WRITE setHost NOTIFY hostChanged)

    Q_OBJECT

public:
    RestConsumer3();
    ~RestConsumer3();

public Q_SLOTS:

    QByteArray host();
    void setHost(QByteArray host);

    void addHeader(QByteArray key, QByteArray value);

    void upload(RestUploader *rest, QByteArray resource, QString file, QString query,bool put);
    void upload(RestUploader *rest, QByteArray resource, QString file, QByteArray data,QString query, bool put);

Q_SIGNALS:
    void hostChanged(QByteArray host);

private Q_SLOTS:
    void parseNetworkResponse(QNetworkReply *reply );

private:
    void setQueryParams(QUrl &url, QString params);
    void addHeaders(QByteArray headers);
    void setHeaders(QNetworkRequest &request);

    MimeTypes mimeTypes;

    QByteArray m_host;
    QHash<QByteArray, QByteArray> headers;

    QNetworkAccessManager networkAccessManager;
    QHash<QNetworkReply* ,RestUploader *> rests;
};

}

#endif // RESTCONSUMER3_H
