#ifndef FILEHELPER_HPP
#define FILEHELPER_HPP
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDebug>
#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniObject>
#include <QJniEnvironment>
#include <QProcess>
#include <jni.h>
#endif

class FileHelper : public QObject
{

 Q_OBJECT

static FileHelper *mInstance;

public:

  FileHelper(QObject *parent = nullptr);

 QString selectedFileName;


static FileHelper *getInstance();

Q_INVOKABLE QString fileNameFromPath(const QString & filePath) const ;
Q_INVOKABLE QString fileSuffixFromPath(const QString & filePath) const;
Q_INVOKABLE  qint64  fileSizeFromPath(const QString & filePath) const;
Q_INVOKABLE  QString  filePath(const QString & filePath) const;
void fileSelected(const QString &url);
Q_INVOKABLE  QString  androidFile();
Q_INVOKABLE  void  androidGallery(QString path);
Q_INVOKABLE void renamefile(QString pathtooldfilename, QString pathtonewfilename);
Q_INVOKABLE  QString  filelocalPath(const QString & filePath) const;
Q_INVOKABLE  QString  dirname(const QString & filePath) const;
Q_INVOKABLE  QString  convertUrlToFilePath(const QUrl &url);
Q_INVOKABLE  QString extractstring(QString firstword,QString criteria);
Q_INVOKABLE  bool testextractstring(QString firstword,QString criteria);
Q_INVOKABLE  QString getfilenameandroid(QString fileUrl);
Q_INVOKABLE  QString getfilepathandroid(QString fileUrl);
Q_INVOKABLE bool copyanyfile(QString copydstfile,QString copysrcfile );
static bool rmDir(const QString &dirPath);
Q_INVOKABLE   void cleardir(QString dirname);
bool checkSomeDirs(QString folder="");
bool checkAnyDir(QString folder="",QString dir="");
bool checkDir(QString dir="");
Q_INVOKABLE  void createdir(QString folder="");
Q_INVOKABLE  void createanydir(QString folder="",QString dir="");






};







#endif // FILEHELPER_HPP







