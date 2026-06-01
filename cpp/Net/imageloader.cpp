

#include "imageloader.hpp"

#include <QUrl>
#include <QDebug>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QSslConfiguration>
#include <qfile.h>
#include <QImage>

/**
 *  This class implements a image loader which will initialize a network request in asynchronous manner.
 *  After receiving response from the network, it saves Image to app data directory.
 *  Then it signals the interested parties about the result.
 */
ImageLoader::ImageLoader(const QString &imageUrl, const QString &filename, QObject* parent) :
    QObject(parent), m_imageUrl(imageUrl), m_filename(filename)
{
}

ImageLoader::~ImageLoader()
{
}

/**
 * QObject* will be stored at request and sent back from response
 *
 */
void ImageLoader::loadImage(QObject* dataObject)
{
    QNetworkAccessManager* netManager = new QNetworkAccessManager(this);

    const QUrl url(m_imageUrl);
    QNetworkRequest request(url);
    // stores the object so we can catch it later
    request.setOriginatingObject(dataObject);



     //SSL AND TLS

    QString sslcert =":/ssl/donate.crt";
    QString pkey = ":/ssl/donate.key";
    QString cacert = ":/ssl/DonaterootCA.crt";

     // ssl cert

     QFile file(sslcert);
     file.open(QIODevice::ReadOnly);
     const QByteArray bytes = file.readAll();
     const QSslCertificate certificate(bytes);

     // ca cert
     QFile fileca(cacert);
     fileca.open(QIODevice::ReadOnly);
     const QByteArray bytesca = fileca.readAll();
     const QSslCertificate certificateca(bytesca);
     // end CA

     // ssl pkey
     QFile filekey(pkey);
     filekey.open(QIODevice::ReadOnly);
     const QByteArray byteskey = filekey.readAll();
     const QSslKey mykey(byteskey,QSsl::Rsa);

     QSslConfiguration config = request.sslConfiguration();
     config.addCaCertificate(certificateca);
     config.setLocalCertificate(certificate);
     config.setPrivateKey(mykey);
     config.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(config);

    QNetworkReply* reply = netManager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}


/**
 * ImageLoader::onReplyFinished()
 *
 * Handler for the signal indicating the response for the previous network request.
 */
void ImageLoader::onReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (reply) {
        if (reply->error() == QNetworkReply::NoError) {
            const qint64 available = reply->bytesAvailable();
            if (available > 0) {
                int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                if(httpStatusCode == 200) {
                    QImage originImage = QImage::fromData(reply->readAll());
                    if(originImage.isNull()) {
                        Q_EMIT loadingFailed(reply->request().originatingObject(), "Cannot construct Image from data: " + m_imageUrl);
                        reply->deleteLater();
                        return;
                    }

                    originImage.save(m_filename);
    //                // this is normaly used for images without any modifications:
    //                QFile dataFile(m_filename);
    //                dataFile.open(QIODevice::WriteOnly);
    //                dataFile.write(data);
    //                dataFile.close();
                    Q_EMIT loaded(reply->request().originatingObject(), originImage.width(), originImage.height());
                } else {
                    QString message;
                    if(httpStatusCode == 301) {
                        message = "redirected to "+reply->header(QNetworkRequest::LocationHeader).toString();

                    } else {
                        message = "Wrong HTTP Status: " + QString::number(httpStatusCode);
                    }
                    Q_EMIT loadingFailed(reply->request().originatingObject(), message);
                }
            }
        } else {
            qDebug() << "ERROR loading image " << m_filename << " " << reply->error() << reply->errorString();
            Q_EMIT loadingFailed(reply->request().originatingObject(), reply->errorString());
        }

        reply->deleteLater();
    }
}
