#ifndef SHARINGUTILS_ANDROID_H
#define SHARINGUTILS_ANDROID_H
/* ************************************************************************** */

#include "SharingUtils.h"

#include <QtGlobal>
#include <QtCore/private/qandroidextras_p.h>

#include <QCoreApplication>
#include <QJniObject>

/* ************************************************************************** */

class AndroidShareUtils : public PlatformShareUtils, public QAndroidActivityResultReceiver
{
    static AndroidShareUtils *mInstance;

    bool mIsEditMode = false;
    qint64 mLastModified = 0;
    QString mCurrentFilePath;

    void processActivityResult(int requestCode, int resultCode);

    QString getExternalFilesDirPath() const;

public:
    AndroidShareUtils(QObject *parent = nullptr);
    static AndroidShareUtils *getInstance();

    bool checkMimeTypeView(const QString &mimeType) override;
    bool checkMimeTypeEdit(const QString &mimeType) override;

    void sendText(const QString &text, const QString &subject, const QUrl &url) override;

    void sendFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId) override;
    void viewFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId) override;
    void editFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId) override;

    void handleActivityResult(int receiverRequestCode, int resultCode, const QJniObject &data) override;
    void onActivityResult(int requestCode, int resultCode);

    void checkPendingIntents(const QString &workingDirPath) override;

public slots:
    void setFileUrlReceived(const QString &url);
    void setFileReceivedAndSaved(const QString &url);
    bool checkFileExits(const QString &url);
};

/* ************************************************************************** */
#endif // SHARINGUTILS_ANDROID_H
