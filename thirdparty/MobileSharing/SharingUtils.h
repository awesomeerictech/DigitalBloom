#ifndef SHARINGUTILS_H
#define SHARINGUTILS_H
/* ************************************************************************** */

#include <QObject>
#include <QString>
#include <QMimeDatabase>
#include <QUrl>
#include <QDebug>

/* ************************************************************************** */

class PlatformShareUtils : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void shareEditDone(int requestCode);
    void shareFinished(int requestCode);
    void shareNoAppAvailable(int requestCode);
    void shareError(int requestCode, QString message);
    void fileUrlReceived(QString url);
    void fileReceivedAndSaved(QString url);

public:
    PlatformShareUtils(QObject *parent = nullptr) : QObject(parent) { };
    virtual ~PlatformShareUtils() = default;

    virtual void checkPendingIntents(const QString &workingDirPath) {
        qDebug() << "checkPendingIntents" << workingDirPath;
    }
    virtual bool checkMimeTypeView(const QString &mimeType) {
        qDebug() << "check view for" << mimeType;
        return true;
    }
    virtual bool checkMimeTypeEdit(const QString &mimeType) {
        qDebug() << "check edit for" << mimeType;
        return true;
    }

    virtual void sendText(const QString &text, const QString &subject, const QUrl &url) {
        qDebug() << text << subject << url.url();
    }
    virtual void sendFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId) {
        qDebug() << filePath << " - " << title << "requestId: " << requestId << " - " << mimeType;
    }
    virtual void viewFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId) {
        qDebug() << filePath << " - " << title << "requestId: " << requestId << " - " << mimeType;
    }
    virtual void editFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId) {
        qDebug() << filePath << " - " << title << "requestId: " << requestId << " - " << mimeType;
    }

    const QMimeDatabase &getMimeDatabase() const {
        return m_mimeDatabase;
    }

private:
    QMimeDatabase m_mimeDatabase;
};

/* ************************************************************************** */

class ShareUtils : public QObject
{
    Q_OBJECT

    PlatformShareUtils *mPlatformShareUtils = nullptr;

Q_SIGNALS:
    void shareEditDone(int requestCode);
    void shareFinished(int requestCode);
    void shareNoAppAvailable(int requestCode);
    void shareError(int requestCode, QString message);
    void fileUrlReceived(QString url);
    void fileReceivedAndSaved(QString url);

public Q_SLOTS:
    void onShareEditDone(int requestCode);
    void onShareFinished(int requestCode);
    void onShareNoAppAvailable(int requestCode);
    void onShareError(int requestCode, const QString &message);
    void onFileUrlReceived(const QString &url);
    void onFileReceivedAndSaved(const QString &url);

public:
    explicit ShareUtils(QObject *parent = nullptr);

    static void registerQML();

    Q_INVOKABLE void checkPendingIntents(const QString &workingDirPath);
    Q_INVOKABLE bool checkMimeTypeView(const QString &mimeType);
    Q_INVOKABLE bool checkMimeTypeEdit(const QString &mimeType);
    const QMimeDatabase &getMimeDatabase() const;

    Q_INVOKABLE void sendText(const QString &text, const QString &subject, const QUrl &url);

    Q_INVOKABLE void sendFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId);
    Q_INVOKABLE void viewFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId);
    Q_INVOKABLE void editFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId);
};

/* ************************************************************************** */
#endif // SHARINGUTILS_H
