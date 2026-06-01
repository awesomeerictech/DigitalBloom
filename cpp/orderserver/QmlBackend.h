#ifndef QMLBACKEND_H
#define QMLBACKEND_H

#include <QObject>
#include <QString>
#include <QDebug>


class QmlBackend : public QObject
{
   
    Q_OBJECT

public:
    explicit QmlBackend (QObject *parent = nullptr);

  
    Q_INVOKABLE void triggerInfoDialog(const QString &message);
    Q_INVOKABLE void triggerDialogError(const QString &message);
    Q_INVOKABLE void triggerDialogToast(const QString &message);

Q_SIGNALS:
    
    void showDialogRequestedInfo(const QString &message);
    void showDialogRequestedError(const QString &message);
    void showDialogRequestedToast(const QString &message);
};

#endif // QMLBACKEND_H
