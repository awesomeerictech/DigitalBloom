#ifndef SHARINGUTILS_IOS_H
#define SHARINGUTILS_IOS_H
/* ************************************************************************** */

#include "SharingUtils.h"

/* ************************************************************************** */

class IosShareUtils : public PlatformShareUtils
{
    Q_OBJECT

public:
    explicit IosShareUtils(QObject *parent = 0);

    bool checkMimeTypeView(const QString &mimeType);
    bool checkMimeTypeEdit(const QString &mimeType);

    void sendText(const QString &text, const QString &subject, const QUrl &url);

    void sendFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId);
    void viewFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId);
    void editFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId);

    void handleDocumentPreviewDone(const int &requestId);

public slots:
    void handleFileUrlReceived(const QUrl &url);
};

/* ************************************************************************** */
#endif // SHARINGUTILS_IOS_H
