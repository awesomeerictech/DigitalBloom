#ifndef SMS_H
#define SMS_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>


class Sms : public QObject
{
    Q_OBJECT
public:
    explicit Sms(QObject *parent = nullptr);

Q_SIGNALS:
    void error(QString msg);

public Q_SLOTS:

   Q_INVOKABLE void singleMessage(QByteArray username, QByteArray to, QByteArray message,QByteArray from,QByteArray bulkSMSMode);

protected Q_SLOTS:
    void finished(QNetworkReply *);
    void sslErrors(QNetworkReply*,QList<QSslError>);

private:
    QNetworkAccessManager _manager;
    static QString  username;
    static QString password;

};

#endif // SMS_H
