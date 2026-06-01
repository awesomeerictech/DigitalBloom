#include "FileHelper.hpp"


FileHelper *FileHelper::mInstance = nullptr;

/* ************************************************************************** */

FileHelper::FileHelper(QObject* parent)
{
    Q_UNUSED(parent)
    // we need the instance for JNI Call
    mInstance = this;
}



FileHelper *FileHelper::getInstance()
{
    if (!mInstance)
    {
        mInstance = new FileHelper;
    }

    return mInstance;
}

  QString FileHelper::fileNameFromPath(const QString & filePath) const {
   return QFileInfo(filePath).fileName();

 }
   QString FileHelper::fileSuffixFromPath(const QString & filePath) const {
     return QFileInfo(filePath).suffix();
   }
  qint64  FileHelper::fileSizeFromPath(const QString & filePath) const {
       qint64 size = 0;
       if(QFileInfo(filePath).size()==0){

         //  QUrl eric = QUrl::fromLocalFile(filePath);
           QUrl eric(filePath);
           QString relative = eric.toString(QUrl::PreferLocalFile);
           qDebug()<< "relative is: " << relative;
           size = QFileInfo(relative).size();


          // size =
       }

       else{


           size = QFileInfo(filePath).size();
       }
     return size;
   }

   QString  FileHelper::filePath(const QString & filePath) const {
       QUrl eric(filePath);
       QString relative = eric.toString(QUrl::PreferLocalFile);
       qDebug()<< "relative is: " << relative;
       return relative;
    }




   void FileHelper::fileSelected(const QString &url) {


       if (url.isEmpty())
          {
              qWarning() << "fileSelected: we got an empty URL";
              return;
          }
          qDebug() << " fileSelected: we got the File URL from JAVA: " << url;
          selectedFileName=url;
          QString myUrl;
          if (url.startsWith("file://"))
          {
              myUrl = url.right(url.length() - 7);
          }
          else
          {
              myUrl = url;
          }

          // check if File exists
          QFileInfo fileInfo = QFileInfo(myUrl);
          if (fileInfo.exists())
          {

          }
          else
          {
              qDebug() << "fileSelected: FILE does NOT exist ";

          }

    }



   void FileHelper::androidGallery(QString path) {


       #ifdef Q_OS_ANDROID


       QJniObject::callStaticMethod<void>("org/stlltd/utils/QtAndroidGallery",
                                                     "scanForPicture",
                                                     "(Ljava/lang/String;)V",
                                                     QJniObject::fromString(path).object<jstring>());



        #endif // Q_OS_ANDROID





   }




   QString  FileHelper::androidFile()  {

      QString results ="";

      #ifdef Q_OS_ANDROID

       selectedFileName = "#";

       /*


        QAndroidJniObject galleryobj;

        QtAndroid::runOnAndroidThreadSync([&galleryobj](){

            galleryobj.callStaticMethod<void>("org/stlltd/utils/QtAndroidGallery",
                                                     "openAnImage",
                                                     "()V");

        });


     */

       QJniObject::callStaticMethod<void>("org/stlltd/utils/QtAndroidGallery",
                                                     "openAnImage",
                                                     "()V");

       while(selectedFileName == "#")
           qApp->processEvents();

           if(QFile(selectedFileName).exists())
           {
               results=selectedFileName;
               qDebug()<<"The file is: " << selectedFileName << Qt::endl;
           }

      #endif // Q_OS_ANDROID

           return results;

   }


   void FileHelper::renamefile(QString pathtooldfilename, QString pathtonewfilename) {


       QFile readFile(pathtonewfilename);
       bool ispresent = readFile.exists();
       if(ispresent) {

          bool ok =readFile.remove();
          if(ok){

              qDebug()<<"deleted existing file" ;
          }
          else {

              qDebug()<<"unable to delete existing file" ;

          }

        }

       else {

           qDebug()<<"proceed to rename";

       }

       if(QFile::rename(pathtooldfilename, pathtonewfilename)) {

           qDebug()<<"file was renamed successfully" << Qt::endl;

       }

       else {

            qDebug()<<"file was not renamed" << Qt::endl;
       }

   }

   QString  FileHelper::filelocalPath(const QString & filePath) const {
      return QFileInfo(filePath).path();
    }

   QString  FileHelper::dirname(const QString & filePath) const {
      return QFileInfo(filePath).filePath();
    }



    QString  FileHelper::convertUrlToFilePath(const QUrl &url) {
     // This function convert the url passed from qml in a valid resource path.
     if (url.scheme().compare(QLatin1String("qrc"), Qt::CaseInsensitive) == 0) {
       if (url.authority().isEmpty()) {
         return QLatin1Char(':') + url.path();
       }
     }
     return QString();
   }

  QString FileHelper::extractstring(QString firstword,QString criteria) {


       QRegularExpression eric;
       QString results="";
       eric.setPattern("(?<="+firstword+").*");
        QRegularExpressionMatch match = eric.match(criteria);
        if (match.hasMatch()) {
              QString matched = match.captured(0);
              qDebug()<<"match is:" << matched;
              results=matched;
          }


       return results;



   }

   bool FileHelper::testextractstring(QString firstword,QString criteria) {


        QRegularExpression eric;
        bool results=false;
        eric.setPattern("(?<="+firstword+").*");
         QRegularExpressionMatch match = eric.match(criteria);
         if (match.hasMatch()) {

               results=true;
           }


        return results;



    }



   QString FileHelper::getfilenameandroid(QString fileUrl) {

        QString filename ="";

       #ifdef Q_OS_ANDROID
        QtJniTypes::Context mycontext = QNativeInterface::QAndroidApplication::context();


       QJniObject uri = QJniObject::callStaticObjectMethod(
            "android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;",
            QJniObject::fromString(fileUrl).object<jstring>());

        filename =
            QJniObject::callStaticObjectMethod(
                "org/stlltd/utils/QSharePathResolver", "getRealPathFromURI",
                "(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;",
                mycontext.object() ,uri.object())
                .toString();

        #endif

        return filename;



   }


   QString FileHelper::getfilepathandroid(QString fileUrl) {

        QString filepath ="";

       #ifdef Q_OS_ANDROID

        QtJniTypes::Context mycontext = QNativeInterface::QAndroidApplication::context();

       QJniObject uri = QJniObject::callStaticObjectMethod(
            "android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;",
            QJniObject::fromString(fileUrl).object<jstring>());

        filepath =
            QJniObject::callStaticObjectMethod(
                "org/stlltd/utils/QSharePathResolver", "getRealPathFromURI",
                "(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;",
                      mycontext.object(),  uri.object())
                .toString();

        #endif

        return filepath;

        // // ef

        // QJniObject jsPath = QJniObject::fromString(filePath);
        // QJniObject jsTitle = QJniObject::fromString(title);
        // QJniObject jsMimeType = QJniObject::fromString(mimeType);

        // jboolean ok = QJniObject::callStaticMethod<jboolean>("org/stlltd/utils/QShareUtils",
        //                                                      "editFile",
        //                                                      "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z",
        //                                                      jsPath.object<jstring>(), jsTitle.object<jstring>(),
        //                                                      jsMimeType.object<jstring>(), requestId);

        // if (!ok)
        // {
        //     qWarning() << "Unable to resolve activity from Java";
        //     Q_EMIT shareNoAppAvailable(requestId);
        // }

        // /ef



   }


   bool FileHelper::copyanyfile(QString copydstfile,QString copysrcfile )
    {




            QFile readFile(copydstfile);
            bool ispresent = readFile.exists();
            if(ispresent) {

               bool ok =readFile.remove();
               if(ok){

                   qDebug()<<"deleted existing file" ;
               }
               else {

                   qDebug()<<"unable to delete existing file" ;
                   return false;
               }

             }

            else {

                qDebug()<<"proceed to copy";

            }

            if(!readFile.exists()) {
                qDebug() << "image data path does not exist: " << copydstfile;
                QFile srcFile( copysrcfile);
                if(srcFile.exists()) {
                    // copy file from src to data path
                    bool copyOk = srcFile.copy(copydstfile);
                    if (!copyOk) {
                        qDebug() << "cannot copy file from src to data path";
                        return false;
                    }

                    copyOk = readFile.setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser);
                    if (!copyOk) {
                        qDebug() << "cannot set Permissions to read / write settings";
                        return false;
                    }
                } else {
                    qDebug() << "nothing in src " << copysrcfile;
                    return false;
                }
            }
            if (!readFile.open(QIODevice::ReadOnly)) {
                qWarning() << "Couldn't open file: " << copydstfile                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ;
                return false;
            }

            return true;
    }

 bool FileHelper::rmDir(const QString &dirPath)
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

void FileHelper::cleardir(QString dirname) {


       rmDir(dirname);

   }

   bool FileHelper::checkSomeDirs(QString folder)
      {
          QDir myDir;
          bool existsimgs;
          bool existsvids;
          QString mDataImagesPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/images/photos";
          QString mDataVideosPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/videos/movies";

          existsimgs = myDir.exists(mDataImagesPath);
          if (!existsimgs) {
              bool ok = myDir.mkpath(mDataImagesPath);
              if(!ok) {
                  qWarning() << "Couldn't create mDataImagesPath " <<mDataImagesPath;
                  return false;
              }
              qDebug() << "created directory mDataImagesPath " << mDataImagesPath;
          }

          existsvids = myDir.exists(mDataVideosPath);
          if (!existsvids) {
              bool ok = myDir.mkpath(mDataVideosPath);
              if(!ok) {
                  qWarning() << "Couldn't create mDataVideosPath " << mDataVideosPath;
                  return false;
              }
              qDebug() << "created directory mDataVideosPath " << mDataVideosPath;
          }

          if(!(folder=="")){

              bool existsfolder;
              QString somePath=QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/"+folder;
              existsfolder = myDir.exists(somePath);
              if (!existsfolder) {
                  bool ok = myDir.mkpath(somePath);
                  if(!ok) {
                      qWarning() << "Couldn't create somePath " <<somePath;
                      return false;
                  }
                  qDebug() << "created directory somePath " << somePath;
              }



          }



          return true;
      }


   bool FileHelper::checkAnyDir(QString folder,QString dir)
      {
          QDir myDir;
          bool existsdir;
          existsdir = myDir.exists(dir);
          if (!existsdir) {
              bool ok = myDir.mkpath(dir);
              if(!ok) {
                  qWarning() << "Couldn't create main directory " <<dir;
                  return false;
              }
              qDebug() << "created main directory " << dir;
          }



          if(!(folder=="")){

              bool existsfolder;
              QString somePath=dir+folder;
              existsfolder = myDir.exists(somePath);
              if (!existsfolder) {
                  bool ok = myDir.mkpath(somePath);
                  if(!ok) {
                      qWarning() << "Couldn't create somePath " <<somePath;
                      return false;
                  }
                  qDebug() << "created directory somePath " << somePath;
              }



          }



          return true;
      }

   void FileHelper::createanydir(QString folder,QString dir){


         bool okdir = checkAnyDir(folder,dir);
                     if(!okdir) {
                         qFatal("Directory could not be created for some reasons");
                     }


     }

   bool FileHelper::checkDir(QString dir) {

       QDir myDir;
       bool existsdir;
       bool exists = true;
       existsdir = myDir.exists(dir);
       if (!existsdir) {
           exists =false;

       }

       return exists;



   }

 void FileHelper::createdir(QString folder){


       bool okdir = checkSomeDirs(folder);
                   if(!okdir) {
                       qFatal("Directory could not be created for some reasons");
                   }


   }

#ifdef Q_OS_ANDROID
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_org_stlltd_digitalbloom_QtAndroidGallery_fileSelected(JNIEnv */*env*/,
                                                     jobject /*obj*/,
                                                     jstring url)
{

   QString myresults = QJniObject(url).toString();
   FileHelper::getInstance()->fileSelected(myresults);

   return;
}

#ifdef __cplusplus
}
#endif
#endif
