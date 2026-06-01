#ifndef SHARINGAPPLICATION_H
#define SHARINGAPPLICATION_H
/* ************************************************************************** */

#include <QGuiApplication>

class QQmlContext;
class ShareUtils;

/* ************************************************************************** */

class SharingApplication : public QGuiApplication
{
    Q_OBJECT

    ShareUtils *mShareUtils = nullptr;
    bool mPendingIntentsChecked = false;

    QString mAppDataFilesPath;
    QString mDocumentsWorkPath;

public:
    explicit SharingApplication(int &argc, char **argv);
    ~SharingApplication();

Q_SIGNALS:
    void fileDropped(const QString &filePath);

public Q_SLOTS:
    void onApplicationStateChanged(Qt::ApplicationState state);

protected:
    bool event(QEvent *e);
};

/* ************************************************************************** */
#endif // SHARINGAPPLICATION_H
