#ifndef RESTUPLOADER_H
#define RESTUPLOADER_H

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
#include <QQueue>
#include "mimetypes.h"
#include "restconsumer3.h"


namespace eric {

class RestUploader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(RestConsumer3* restConsumer READ getRestConsumer WRITE setRestConsumer)
    Q_PROPERTY(int current READ getCurrent NOTIFY currentChanged)
    Q_PROPERTY(int total READ getTotal NOTIFY totalChanged)

public:
    RestUploader();
    ~RestUploader();


public Q_SLOTS:
    void upload(QByteArray resource, QString file,QString query, bool put = false);
    void upload(QByteArray resource, QString file, QByteArray data,QString query, bool put = false);

    void upload(QByteArray resource, QList<QString> files, QString query,bool put = false);
    void upload(QByteArray resource, QList<QString> files, QByteArray data,  QString query,bool put = false);

    void setRestConsumer(RestConsumer3 *consumer);
    RestConsumer3* getRestConsumer();
    void myuploadprogress(qint64 sent, qint64 total);


    int getCurrent();
    int getTotal();

Q_SIGNALS:

    void uploaded(const QByteArray rawData);
    void progress(qint64 sent, qint64 total);
    void finished(int total);

    void currentChanged(int value);
    void totalChanged(int value);

    void networkError( QNetworkReply::NetworkError err );
    void serverError(const QByteArray err);

    // errors issued by RestConsumer before sending a request
    void error(const QByteArray err);

    void destroying(RestUploader *rest);

private:
    void uploadMore();

private:
    QQueue<QString> pending_files;
    bool is_put;
    QByteArray resource;
    int current;
    int total;
    QString urlquery;
    RestConsumer3 *m_restConsumer = nullptr;
};

}

#endif // RESTUPLOADER_H
