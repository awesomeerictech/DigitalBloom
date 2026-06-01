#include "QmlBackend.h"

// The implementation of the class methods is done inline in the header file above.
// You usually don't need much in this .cpp file for a simple signal Q_EMITter.


//If you wanted to move the implementation out of the header:

QmlBackend::QmlBackend(QObject *parent) : QObject(parent) {
    qDebug() << "QmlBackend initialized.";
}

void QmlBackend::triggerInfoDialog(const QString &message) {
    qDebug() << "C++ triggered: Q_EMIT showDialogRequestedInfo signal.";
    Q_EMIT showDialogRequestedInfo(message);
}

void QmlBackend::triggerDialogError(const QString &message) {
    qDebug() << "C++ triggered: Q_EMIT showDialogRequestedError signal.";
    Q_EMIT showDialogRequestedError(message);
}

void QmlBackend::triggerDialogToast(const QString &message) {
    qDebug() << "C++ triggered: Q_EMIT showDialogRequestedToast signal.";
    Q_EMIT showDialogRequestedToast(message);
}



