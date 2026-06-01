#ifndef COPYASSETS_H
#define COPYASSETS_H
#include <QObject>
#include <QQmlEngine>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QtGlobal>

class CopyAssets : public QObject
{
  Q_OBJECT

public:


    static bool cpDir(const QString &srcPath, const QString &dstPath)
      {
      rmDir(dstPath);

       //   qDebug()<<"dst file name " << QFileInfo(dstPath).fileName() ;
       //   qDebug()<<"dst file path " << QFileInfo(dstPath).path();
          QString fulldirs =  QFileInfo(dstPath).path()+ "/" + QFileInfo(dstPath).fileName();
          QString fullpaths =QFileInfo(dstPath).path();
         // qDebug()<<"full dirs with no sub dirs are " << fullpaths;
         //  qDebug()<<"full dirs with sub dirs are " << fulldirs;
           bool ok = createDir(fulldirs);
                       if(!ok) {
                            return false;
                        //   qFatal("App won't work - cannot create directory");

                       }

          QDir srcDir(srcPath);
          Q_FOREACH(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
              QString srcItemPath = srcPath + "/" + info.fileName();
             // qDebug()<<"src item path" << srcItemPath;
              //QString dstItemPath = fullpaths + "/" + info.fileName();
              QString dstItemPath = dstPath + "/" + info.fileName();
              if (info.isDir()) {
                  if (!cpDir(srcItemPath, dstItemPath)) {
                      return false;
                  }
                //  qDebug()<<"recursion" ;
              } else if (info.isFile()) {
                 QString mdataPathfile = dstItemPath;
                  QFile readFile(mdataPathfile);
                              if(!readFile.exists()) {
                                 // qDebug() << "copy file path does not exist: " << mdataPathfile;
                                  QFile qrcFile(srcItemPath);
                                  if(qrcFile.exists()) {
                                      // copy file from qrc to data
                                      bool copyOk = qrcFile.copy(mdataPathfile);
                                      if (!copyOk) {
                                       //   qDebug() << "cannot copy data assets from qrc to data path";
                                          return false;
                                      }
                                      // IMPORTANT !!! copying from RESOURCES ":/data-assets/" to AppDataLocation
                                      // makes the target file READ ONLY - you must set PERMISSIONS
                                      // copying from RESOURCES ":/config/" to GenericDataLocation the target is READ-WRITE
                                      copyOk = readFile.setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser);
                                      if (!copyOk) {
                                      //    qDebug() << "cannot set Permissions to read / write settings";
                                          return false;
                                      }
                                  } else {
                                    //  qDebug() << "nothing in qrc " << srcItemPath;
                                      return false;
                                  }
                              }
                              if (!readFile.open(QIODevice::ReadOnly)) {
                                //  qWarning() << "Couldn't open file: " << mdataPathfile;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ;
                                  return false;
                              }

               //   setpermissionuser(dstItemPath);
               //   setpermissionowner(dstItemPath);
                //  setpermissiongroup(dstItemPath);
              } else {
                //  qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
              }
          }

          return true;

      }


    static bool rmDir(const QString &dirPath)
         {
             QDir dir(dirPath);
             if (!dir.exists())
                 return true;
             Q_FOREACH(const QFileInfo &info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
                 if (info.isDir()) {
                     if (!rmDir(info.filePath()))
                         return false;
                 } else {
                     if (!dir.remove(info.fileName()))
                         return false;
                 }
             }
             QDir parentDir(QFileInfo(dirPath).path());
             return parentDir.rmdir(QFileInfo(dirPath).fileName());
         }


     static bool createDir(QString anydir)
         {
             QDir myDir;
             bool exists;
             exists = myDir.exists( anydir);
             if (!exists) {
                 bool ok = myDir.mkpath( anydir);
                 if(!ok) {
                     qWarning() << "Couldn't create  anydir " <<  anydir;
                     return false;
                 }
                 qDebug() << "created directory  anydir " <<  anydir;
             }


             return true;
         }






    void setenvssl()  {

   #if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)

   bool ok = qunsetenv("ANDROID_OPENSSL_SUFFIX");

   if(!ok) {
       QString suff =  "_3";

       qputenv("ANDROID_OPENSSL_SUFFIX",suff.toUtf8());

   }

   else {

       qunsetenv("ANDROID_OPENSSL_SUFFIX");
       QString suff =  "_3";

       qputenv("ANDROID_OPENSSL_SUFFIX",suff.toUtf8());

   }

   #endif


           }


   static void setpermissionowner(QString path) {


       QFile readFile(path);
       bool exists = readFile.exists();
       exists= readFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
       if (!exists) {
       qDebug() << "cannot set Permissions to read owner / write owner";
       return;
        }
    // QFileDevice::Permissions p = QFile(path).permissions();
     // qDebug()<<"All permissions are: " << p;
    }


   static void setpermissionuser(QString file){


    QFile readFile(file);
    bool exists = readFile.exists();
    exists= readFile.setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser);
    if (!exists) {
    qDebug() << "cannot set Permissions to read user / write user";
    return;
     }


   }

   static void setpermissiongroup(QString file){


    QFile readFile(file);
    bool exists = readFile.exists();
    exists= readFile.setPermissions(QFileDevice::ReadGroup | QFileDevice::WriteGroup);
    if (!exists) {
    qDebug() << "cannot set Permissions to read group / write group";
    return;
     }


   }





};

#endif // COPYASSETS_H
