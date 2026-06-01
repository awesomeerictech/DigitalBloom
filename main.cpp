
#include "cpp/tools/DatabaseManager.h"
#include "cpp/tools/SettingsManager.h"
#include "cpp/tools/BarcodeManager.h"
#include "cpp/tools/utils_camera.h"
#include "cpp/tools/SystrayManager.h"
#include "cpp/tools/NotificationManager.h"

#include <utils_app.h>
#include <utils_screen.h>
#include <utils_clipboard.h>
#include "utils_language.h"
#include <utils_fpsmonitor.h>

#include <MobileUI>
#include <MobileSharing>
#include <SingleApplication>

#if defined(qzxing)
#include <QZXing>
#endif

#if defined(zxingcpp)
#include <ZXingQt>
#endif

#if defined(zint)
#include <ZintQml>
#endif

#include <Qaterial/Qaterial.hpp>
#include <QApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSslCertificate>
#include <QSsl>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QSslKey>
#include <QSslCertificateExtension>
#include <QSslCipher>
#include <QSslDiffieHellmanParameters>
#include <QSslEllipticCurve>
#include <QSslError>
#include <QSslPreSharedKeyAuthenticator>
#include <QFile>
#include <QFileInfo>
#include <QQuickWindow>
#include <QQmlContext>
#include <QtGui/QScreen>
#include <QQuickWindow>
#include <Qt>
#include <QDebug>
#include <QPermissions>
#include <QLoggingCategory>
#include <Qaterial/Qaterial.hpp>
#include "cpp/Net/gu.h"
#include <QtGlobal>
#include <QTranslator>
#include <QLibraryInfo>
#include <QIcon>
#include <SunAndMoon>
#include <QQuickStyle>

#if defined(Q_OS_ANDROID)
#include "cpp/tools/AndroidService.h"
#include "private/qandroidextras_p.h" // for QAndroidService
#endif

#include "CopyAssets.h"
#include "EricType.hpp"
#include "MarkerModel.hpp"
#include <QLibraryInfo>
#include <QVersionNumber>
#include <QSurfaceFormat>
#include "../../qtstatusbar/src/statusbar.h"
#include <QtQml/qqml.h>
#include "cpp/Net/gu.h"
#include <QQuickStyle>
// include server controller
#include "orderserver/servercontrollerwt.h" // line 669
#include "orderserver/AssetExtractor.h"
#include "orderserver/LibArchiveQmlBridge.h"
#include <http/Android.h>
#include <QLoggingCategory>
#include <cstdlib>
#include "Util/logger.h"
#include "tools/ServerManager.h"
#include "orderserver/DataUploader.h"
#include "orderserver/WrapUpload.h"
#include "orderserver/TimedUploader.h"
#include "orderserver/QmlBackend.h"

/* ************************************************************************** */


#ifdef Q_OS_ANDROID
    bool checkPermission() {
    QList<bool> permissions;    auto r = QtAndroidPrivate::checkPermission("android.permission.READ_EXTERNAL_STORAGE").result();
    if (r != QtAndroidPrivate::Authorized)
    {
        r = QtAndroidPrivate::requestPermission("android.permission.READ_EXTERNAL_STORAGE").result();
        if (r == QtAndroidPrivate::Denied)
            permissions.append(false);
    }
    r = QtAndroidPrivate::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE").result();
    if (r != QtAndroidPrivate::Authorized)
    {
        r = QtAndroidPrivate::requestPermission("android.permission.WRITE_EXTERNAL_STORAGE").result();
        if (r == QtAndroidPrivate::Denied)
            permissions.append(false);
    }
    r = QtAndroidPrivate::checkPermission("android.permission.READ_MEDIA_IMAGES").result();
    if (r != QtAndroidPrivate::Authorized)
    {
        r = QtAndroidPrivate::requestPermission("android.permission.READ_MEDIA_IMAGES").result();
        if (r == QtAndroidPrivate::Denied)
            permissions.append(false);
    }
    return (permissions.count() != 3);
}
#endif


#if defined(Q_OS_ANDROID)
void accessAllFiles()
{
    if(QOperatingSystemVersion::current() < QOperatingSystemVersion(QOperatingSystemVersion::Android, 11)) {
        qDebug() << "it is less then Android 11 - ALL FILES permission isn't possible!";
        return;
    }
// Here you have to set your PackageName
#define PACKAGE_NAME "package:org.stlltd.digitalbloom"
    jboolean value = QJniObject::callStaticMethod<jboolean>("android/os/Environment", "isExternalStorageManager");
    if(value == false) {
        qDebug() << "requesting ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION";
        QJniObject ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION = QJniObject::getStaticObjectField( "android/provider/Settings", "ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION","Ljava/lang/String;" );
        QJniObject intent("android/content/Intent", "(Ljava/lang/String;)V", ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION.object());
        QJniObject jniPath = QJniObject::fromString(PACKAGE_NAME);
        QJniObject jniUri = QJniObject::callStaticObjectMethod("android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;", jniPath.object<jstring>());
        QJniObject jniResult = intent.callObjectMethod("setData", "(Landroid/net/Uri;)Landroid/content/Intent;", jniUri.object<jobject>() );
        QtAndroidPrivate::startActivity(intent, 0);
    } else {
        qDebug() << "SUCCESS ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION";
    }
}

// Function to write a file to a shared location (e.g., Downloads folder)
void writeToSharedStorage() {
    // 1. Verify permission status again just in case this function is called directly.
    jboolean isManager = QJniObject::callStaticMethod<jboolean>("android/os/Environment", "isExternalStorageManager");
    if (isManager == false) {
        qDebug() << "Cannot write: MANAGE_EXTERNAL_STORAGE permission not granted.";
        return;
    }

    // 2. Get the path to a common public directory (e.g., Home location)
    // You must use QStandardPaths to find the correct system-specific path.
    QString dbPath_one = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    if (dbPath_one.isEmpty()) {
        qDebug() << "Could not find a writable Downloads location.";
        return;
    }

    // Ensure the directory exists (QStandardPaths usually ensures this for standard locations)
    QDir dir(dbPath_one);
    if (!dir.exists()) {
        dir.mkpath(dbPath_one);
    }

    // 3. Define the file path
    const QString fileName = dbPath_one + "/data.db";
    QFile file(fileName);

    bool exists= file.setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser);
    if (!exists) {
        qDebug() << "File does not exist writing it now";
        // 4. Write to the file
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write("This file was written using Qt 6 with MANAGE_EXTERNAL_STORAGE permission.");
            file.close();
            qDebug() << "Successfully wrote to:" << fileName;
        } else {
            qDebug() << "Failed to open file for writing:" << file.errorString();
        }
    }

    else {

        qDebug() << "File already exists, exiting..." << Qt::endl;
    }


}

#endif






#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_LINUX)


#else

#endif


#ifdef Q_OS_ANDROID

#endif

// ──── FUNCTIONS ────

#include "cpp/translationsmanager.h"
#include "cpp/system.h"

static QObject *
system_manager_singletontype_provider(QQmlEngine *engine,
                                      QJSEngine *scriptEngine) {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    auto system = new System();
    return system;
}

static QObject *
translations_manager_singletontype_provider(QQmlEngine *engine,
                                            QJSEngine *scriptEngine) {
    Q_UNUSED(scriptEngine)
    auto translationsManager = new TranslationsManager();

    QObject::connect(translationsManager, SIGNAL(currentLanguageChanged()),
                     engine, SLOT(retranslate()));

    return translationsManager;
}



void registerTypes(){


    eric::Gu::registerTypes();

}


bool checkDirs()
{
    QDir myDir;
    bool exists;
    QString mDataRoot = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);

    exists = myDir.exists(mDataRoot);
    if (!exists) {
        bool ok = myDir.mkpath(mDataRoot);
        if(!ok) {
            qWarning() << "Couldn't create mDataRoot " << mDataRoot;
            return false;
        }
        qDebug() << "created directory mDataRoot " << mDataRoot;
    }



    return true;
}

bool checkCamDirs()
{
    QDir myDir;
    bool existsimg;
    bool existsvid;
    bool existsimgs;
    bool existsvids;
    bool existsextra;
    QString mDataCameraimgRoot = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/camera/images";
    QString mDataCameravidRoot = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/camera/video";
    QString mDataImagesPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/images/photos";
    QString mDataVideosPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/videos/movies";
    QString mExtraimgRoot = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/images/extra";


    existsimg = myDir.exists(mDataCameraimgRoot);
    if (!existsimg) {
        bool ok = myDir.mkpath(mDataCameraimgRoot);
        if(!ok) {
            qWarning() << "Couldn't create mDataCameraimgRoot " << mDataCameraimgRoot;
            return false;
        }
        qDebug() << "created directory mDataCameraimgRoot " << mDataCameraimgRoot;
    }

    existsvid = myDir.exists(mDataCameravidRoot);
    if (!existsvid) {
        bool ok = myDir.mkpath(mDataCameravidRoot);
        if(!ok) {
            qWarning() << "Couldn't create mDataCameravidRoot " << mDataCameravidRoot;
            return false;
        }
        qDebug() << "created directory mDataCameravidRoot " << mDataCameravidRoot;
    }

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


    existsextra = myDir.exists(mExtraimgRoot);
    if (!existsextra ) {
        bool ok = myDir.mkpath(mExtraimgRoot);
        if(!ok) {
            qWarning() << "Couldn't create mExtraimgRoot " << mExtraimgRoot;
            return false;
        }
        qDebug() << "created directory mExtraimgRoot " << mExtraimgRoot;
    }



    return true;
}





void defaultssl() {



    QString sslcert =":/ssl/donate.crt";
    QString pkey = ":/ssl/donate.key";
    QString cacert = ":/ssl/DonaterootCA.crt";

    // ssl cert

    QFile filecert(sslcert);
    if (!filecert.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open SSL certificate file:" << sslcert << filecert.errorString();
        return; // Exit or handle the error
    }
    const QByteArray bytes = filecert.readAll();
    const QSslCertificate certificate(bytes);

    // ca cert
    QFile fileca(cacert);
    if (!fileca.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open CA certificate file:" << cacert << fileca.errorString();
        return;
    }
    const QByteArray bytesca = fileca.readAll();
    const QSslCertificate certificateca(bytesca);
    // end CA

    // ssl pkey
    QFile filekey(pkey);
    if (!filekey.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open ssl private key file:" << pkey << filekey.errorString();
        return;
    }
    const QByteArray byteskey = filekey.readAll();
    const QSslKey mykey(byteskey,QSsl::Rsa);



    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.addCaCertificate(certificateca);
    config.setLocalCertificate(certificate);
    config.setPrivateKey(mykey);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setDefaultConfiguration(config);

}


// We make the backend handler accessible globally or pass it around as needed
QmlBackend globalBackendHandler;

using namespace toolkit;

/* ************************************************************************** */

int main(int argc, char *argv[])
{
    // Arguments parsing ///////////////////////////////////////////////////////

   QLoggingCategory::setFilterRules("*.info=false\n*.warning=false\n*.debug=false");


    // Logger::Instance().add(std::make_shared<ConsoleChannel>());
    // Logger::Instance().add(std::make_shared<FileChannel>(download_dir));
    // Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    // Compose a std::string path for FileChannel constructor
    QString m_log = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/logs";
    if (!QDir().mkpath(m_log)) {
        qDebug() << "Failed to create directory:" << m_log;
    }
    std::string download_dir = m_log.toUtf8().constData();
    std::string fileChannelPath = download_dir;
    Logger::Instance().add(std::make_shared<ConsoleChannel>("DigitalBloom",toolkit::LError));
    Logger::Instance().add(std::make_shared<EventChannel>("DigitalBloom", toolkit::LError));
    // Prefer creating FileChannel with explicit path if FileChannel supports it.
    // toolkit::LTrace
    // LogPriorityArr[LTrace] = ANDROID_LOG_VERBOSE;
    // LogPriorityArr[LDebug] = ANDROID_LOG_DEBUG;
    // LogPriorityArr[LInfo] = ANDROID_LOG_INFO;
    // LogPriorityArr[LWarn] = ANDROID_LOG_WARN;
    // LogPriorityArr[LError] = ANDROID_LOG_ERROR;
    // s_syslog_lev[LTrace] = LOG_DEBUG;
    // s_syslog_lev[LDebug] = LOG_INFO;
    // s_syslog_lev[LInfo] = LOG_NOTICE;
    // s_syslog_lev[LWarn] = LOG_WARNING;
    // s_syslog_lev[LError] = LOG_ERR;
    // ZLToolKit FileChannel commonly offers a ctor accepting a base path string.
    // If your FileChannel header exposes a different API, please tell me and I will adapt.
    auto fc = std::make_shared<FileChannel>("DigitalBloom",fileChannelPath, toolkit::LError);
    fc->setPath(fileChannelPath);
    Logger::Instance().add(fc);
    // Use async writer to avoid blocking IO from the logging thread
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());
    // Optional: write startup message
    InfoL << "Logger initialized. Logs -> " << fileChannelPath;

// At program startup, before Wt server / Wt::WApplication creations:
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_LINUX)
    // Choose AppDataLocation (internal app storage) or CacheLocation
    QString wtTmpDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (wtTmpDir.isEmpty()) {
        wtTmpDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }
    wtTmpDir = QDir(wtTmpDir).filePath("wt_tmp");   // e.g. .../files/yourapp/wt_tmp

    // Ensure directory exists
    if (!QDir(wtTmpDir).exists()) {
        if (!QDir().mkpath(wtTmpDir)) {
            qWarning() << "Failed to create WT tmp dir:" << wtTmpDir;
        }
    }

    // Set environment variable for Wt (both Qt and POSIX variants to be safe)
    bool succ = qputenv("WT_TMP_DIR", wtTmpDir.toUtf8());
    if(succ)
        qInfo() << "we set WT_TMP_DIR successfully" << Qt::endl;


    qInfo() << "WT_TMP_DIR set to:" << wtTmpDir;
#else
    // Optionally set WT_TMP_DIR on other platforms if you want a custom location
    // qputenv("WT_TMP_DIR", "/some/other/tmp/path");
#endif

    bool ok = checkDirs();
    if(!ok) {
        qFatal("App won't work - cannot create data directory");
    }

    CopyAssets ericassets;
    QString massetsqrc =":/data-assets";
    QString configqrc =":/data-assets/config";
    QString jsonqrc = ":/data-assets/json";
    QString homedir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
    QString configdir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/config";
    QString jsondir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/json";
    ericassets.cpDir(massetsqrc,homedir);
    ericassets.cpDir(configqrc,configdir);
    ericassets.cpDir(jsonqrc,jsondir);


    bool okcam = checkCamDirs();
    if(!okcam) {
        qFatal("Camera will write images and videos elsewhere which is undefined behaviour");
    }

    bool start_minimized = false;
    bool refresh_only = false;
    bool background_service = false;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i])
        {
            //qDebug() << "> arg >" << argv[i];

            if (QString::fromLocal8Bit(argv[i]) == "--start-minimized")
                start_minimized = true;
            if (QString::fromLocal8Bit(argv[i]) == "--service")
                background_service = true;
            if (QString::fromLocal8Bit(argv[i]) == "--refresh")
                refresh_only = true;
        }
    }




    // Background service application //////////////////////////////////////////

    // Refresh data in the background, without starting the UI, then exit
    if (refresh_only)
    {
        QCoreApplication app(argc, argv);
        app.setApplicationName("DigitalBloom");
        app.setOrganizationName("stllimited");
        app.setOrganizationDomain("org");
        CopyAssets ericassets;
        QString massetsqrc =":/data-assets";
        QString configqrc =":/data-assets/config";
        QString jsonqrc = ":/data-assets/json";
        QString homedir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
        QString configdir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/config";
        QString jsondir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/json";
        ericassets.cpDir(massetsqrc,homedir);
        ericassets.cpDir(configqrc,configdir);
        ericassets.cpDir(jsonqrc,jsondir);
#ifdef Q_OS_ANDROID
       accessAllFiles();
       writeToSharedStorage();
#endif

        bool okcam = checkCamDirs();
        if(!okcam) {
            qFatal("Camera will write images and videos elsewhere which is undefined behaviour");
        }




        return app.exec();
    }

    // Android daemon
    if (background_service)
    {
#if defined(Q_OS_ANDROID)
        QAndroidService app(argc, argv);
        app.setApplicationName("DigitalBloom");
        app.setOrganizationName("stllimited");
        app.setOrganizationDomain("org");
        CopyAssets ericassets;
        QString massetsqrc =":/data-assets";
        QString configqrc =":/data-assets/config";
        QString jsonqrc = ":/data-assets/json";
        QString homedir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
        QString configdir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/config";
        QString jsondir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/json";
        ericassets.cpDir(massetsqrc,homedir);
        ericassets.cpDir(configqrc,configdir);
        ericassets.cpDir(jsonqrc,jsondir);

#ifdef Q_OS_ANDROID

#endif

        bool okcam = checkCamDirs();
        if(!okcam) {
            qFatal("Camera will write images and videos elsewhere which is undefined behaviour");
        }

        SettingsManager *sm = SettingsManager::getInstance();
        if (sm && sm->getSysTray())
        {
            AndroidService *as = new AndroidService();
            if (!as) return EXIT_FAILURE;

            return app.exec();
        }

        return EXIT_SUCCESS;
#endif
    }

    // Hacks ///////////////////////////////////////////////////////////////////

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    // NVIDIA driver suspend&resume hack
    auto format = QSurfaceFormat::defaultFormat();
    format.setOption(QSurfaceFormat::ResetNotification);
    QSurfaceFormat::setDefaultFormat(format);
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    // Qt 6.6+ mouse wheel hack
    qputenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", "8000");
#endif

    // GUI application /////////////////////////////////////////////////////////

    SingleApplication app(argc, argv, true);

    // Application name
    app.setApplicationName("DigitalBloom");
    app.setOrganizationName("stllimited");
    app.setOrganizationDomain("org");
    app.setApplicationDisplayName("DigitalBloom");
	
    #if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
   accessAllFiles();
   writeToSharedStorage();
    #else

    #endif

   // order server

    qmlRegisterType<AssetExtractor>("com.server.archive", 1, 0, "AssetExtractor");



    // Init app components
    SettingsManager *sm = SettingsManager::getInstance();
    DatabaseManager *db = DatabaseManager::getInstance();
    NotificationManager *nm = NotificationManager::getInstance();
    if (!sm || !db || !nm)
    {
        qWarning() << "Cannot init app components!";
        return EXIT_FAILURE;
    }



#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    ShareUtils *utilsShare = new ShareUtils();
#else
    QIcon appIcon(":/myassets/gfx/logos/logo.svg");
    app.setWindowIcon(appIcon);

    SystrayManager *st = SystrayManager::getInstance();
#endif

    // Sun and Moon
    SunAndMoon sam;
    if (sm->getSunAndMoon())
    {
        sam.set(sm->getLatitude(), sm->getLongitude(), QDateTime::currentDateTime());
    }

    BarcodeManager *bch = BarcodeManager::getInstance();
    if (!bch) return EXIT_FAILURE;

    // Init generic utils
    UtilsApp *utilsApp = UtilsApp::getInstance();
    UtilsScreen *utilsScreen = UtilsScreen::getInstance();
    UtilsLanguage *utilsLanguage = UtilsLanguage::getInstance();
    UtilsClipboard *utilsClipboard = new UtilsClipboard();
    if (!utilsClipboard) return EXIT_FAILURE;

    UtilsCamera *utilsCamera = UtilsCamera::getInstance();

    if (!utilsCamera) return EXIT_FAILURE;

    if (!utilsApp || !utilsScreen || !utilsLanguage)
    {
        qWarning() << "Cannot init generic utils!";
        return EXIT_FAILURE;
    }

    bool qtConnectivityPatched = false;
#if defined(QT_CONNECTIVITY_PATCHED)
    qtConnectivityPatched = true;
#endif

    MobileUI::registerQML();

    // mine
    qmlRegisterType<StatusBar>("StatusBar", 0, 1, "StatusBar");
    defaultssl();
    registerTypes();
    ericassets.setenvssl();
    EricType mytype;
    MarkerModel model;



    // Translate the application
    utilsLanguage->loadLanguage(sm->getAppLanguage());

    // Start the application
    QQmlApplicationEngine engine;
    QQmlContext *engine_context = engine.rootContext();

    engine_context->setContextProperty("settingsManager", sm);
    engine_context->setContextProperty("databaseManager", db);
    engine_context->setContextProperty("notificationManager", nm);
    engine_context->setContextProperty("markerModel", &model);


    engine_context->setContextProperty("sunAndMoon", &sam);

    engine_context->setContextProperty("utilsApp", utilsApp);
    engine_context->setContextProperty("utilsScreen", utilsScreen);
    engine_context->setContextProperty("utilsLanguage", utilsLanguage);

    engine_context->setContextProperty("startMinimized", (start_minimized || sm->getMinimized()));
    engine_context->setContextProperty("qtConnectivityPatched", qtConnectivityPatched);
    engine_context->setContextProperty("qmlbackend", &globalBackendHandler);


    // Mobile UI
    qmlRegisterType<MobileUI>("MobileUI", 1, 0, "MobileUI");


    engine_context->setContextProperty("barcodeManager", bch);
    engine_context->setContextProperty("utilsCamera", utilsCamera);
    engine_context->setContextProperty("utilsClipboard", utilsClipboard);
    mytype.addContextProperty(engine_context);

    // ──── LOAD AND REGISTER QML ────

    // Important to find 'import Qaterial 1.0'

    qmlRegisterSingletonType<System>(
        "System", 1, 0, "System",
        system_manager_singletontype_provider);

    engine.addImportPath(QStringLiteral("qrc:/"));
    engine.addImportPath(QStringLiteral("qrc:/qml/"));
    engine.addImportPath("qrc:///");
    engine.addImportPath("qrc:///qml");
    engine.addImportPath("qrc:///qml/assets");
    engine.addImportPath("qrc:///assets");


    qmlRegisterType<LibArchiveQmlBridge>("Local.LibArchive", 1, 0, "LibArchiveBridge");
    LibArchiveQmlBridge *lqb = LibArchiveQmlBridge::getInstance();
    DataUploader *du = DataUploader::getInstance();
    WrapUpload *wru = new WrapUpload(du);
    TimedUploader *tup = new TimedUploader(nullptr,du,nullptr);
    if(lqb && du && wru && tup) {

         engine_context->setContextProperty("archiveBridge", lqb);
         engine_context->setContextProperty("netuploader", wru);
         engine_context->setContextProperty("timeduploader", tup);
         qInfo() << "Types were registered" << Qt::endl;

    }

    else {

        qInfo() << "Types were not registered" << Qt::endl;
    }

#ifndef Q_OS_ANDROID
    // 1) register QML types if you want to expose the server objects to QML
    ServerManager::registerQmlTypes();

    // 2) create/ensure singleton instance (optional; startAllServers will also create if needed)
    ServerManager* mgr = ServerManager::instance();
    mgr->mycontext(engine_context);
    // Non-Android: start servers directly under app lifecycle
    qDebug() << "Starting native servers (non-Android)";
    mgr->startAllServers();
#else
    // Android: register BroadcastReceiver and start QtService instead
    AndroidService androidService;
    androidService.setcontext(engine_context);
    androidService.service_registerCommService();
#endif


#if QT_CONFIG(permissions)
    if (qApp->checkPermission(QCameraPermission{}) != Qt::PermissionStatus::Granted) {
        qApp->requestPermission(QCameraPermission{}, [](const QPermission &permission) {
            if (permission.status() != Qt::PermissionStatus::Granted) {
                qWarning() << "Impossible to get Camera permission!";
            }
        });
    }
#endif

#if defined(qzxing)
    // Barcode (QZXing)
    QZXing::registerQMLTypes();
    QZXing::registerQMLImageProvider(engine);
#endif

#if defined(zxingcpp)
    // Barcode (zxing-cpp)
    ZXingQt::registerQMLTypes();
    ZXingQt::registerQMLImageProvider(engine);
#endif

#if defined(zint)
    // Barcode generator (zint-qml)
    ZintQml::registerQMLTypes();
    ZintQml::registerQMLImageProvider(engine);
#endif


    // Load Qaterial
    qaterial::loadQmlResources();
    qaterial::registerQmlTypes();

    // Load the main view
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(FORCE_MOBILE_UI)
    engine_context->setContextProperty("utilsShare", utilsShare);
    engine.loadFromModule("DigitalBloom", "MobileApplication");
#else
    engine_context->setContextProperty("systrayManager", st);
    engine.loadFromModule("DigitalBloom", "DesktopApplication");
#endif

    if (engine.rootObjects().isEmpty())
    {
        qWarning() << "Cannot init QmlApplicationEngine!";
        return EXIT_FAILURE;
    }

    // For i18n retranslate
    utilsLanguage->setQmlEngine(&engine);

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS) // desktop section

    // QQuickWindow must be valid at this point
    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
    engine_context->setContextProperty("quickWindow", window);
    FrameRateMonitor *utilsFpsMonitor = new FrameRateMonitor(window);
    engine_context->setContextProperty("utilsFpsMonitor", utilsFpsMonitor);



    // React to secondary instances
    QObject::connect(&app, &SingleApplication::instanceStarted, window, &QQuickWindow::show);
    QObject::connect(&app, &SingleApplication::instanceStarted, window, &QQuickWindow::raise);

    // Systray?
    st->setupSystray(window);
    if (sm->getSysTray()) st->installSystray();


#if defined(Q_OS_MACOS)
    // macOS dock
    MacOSDockHandler *dockIconHandler = MacOSDockHandler::getInstance();
    dockIconHandler->setupDock(window);
    engine_context->setContextProperty("utilsDock", dockIconHandler);
#endif

#endif // desktop section

#if defined(Q_OS_ANDROID)
    QNativeInterface::QAndroidApplication::hideSplashScreen(333);
   // if (sm->getSysTray()) AndroidService::service_start();
    androidService.service_start(); //AndroidService::service_start();
#endif

   // DatabaseManager dbManager; // Assuming a DatabaseManager instance
  //  QObject::connect(&app, &QCoreApplication::aboutToQuit, &dbManager, &DatabaseManager::closeDatabase);

    return app.exec();
}

/* ************************************************************************** */
