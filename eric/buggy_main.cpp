// ──── INCLUDE ────

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
#include <QLoggingCategory>
#include <Qaterial/Qaterial.hpp>
#include "cpp/Net/gu.h"
#include <QtGlobal>
#include <QTranslator>
#include <QLibraryInfo>
#include <QIcon>
#if defined(qzxing)
#include <QZXing>
#endif

#if defined(zxingcpp)
#include <ZXingQt>
#endif

#if defined(zint)
#include <ZintQml>
#endif
#include "CopyAssets.h"
#include "EricType.hpp"
#include "MarkerModel.hpp"
#include <QLibraryInfo>
#include <QVersionNumber>
#include <QSurfaceFormat>
#include "../../qtstatusbar/src/statusbar.h"
#include <QtQml/qqml.h>
// Force the static plugin to be linked and registered

#include "tools/DatabaseManager.h"
#include "tools/SettingsManager.h"
#include "tools/SystrayManager.h"
#include "tools/NotificationManager.h"


#include "utils_app.h"
#include "utils_screen.h"
#include "utils_language.h"
#include <utils_clipboard.h>
#include <utils_fpsmonitor.h>
#include "cpp/tools/BarcodeManager.h"
#include "cpp/tools/utils_camera.h"

#if defined(Q_OS_MACOS)
#include "utils_os_macos_dock.h"
#endif

#include <MobileUI>
#include <MobileSharing>
#include <SingleApplication>
#include <SunAndMoon>
#include <QQuickStyle>




#if defined(Q_OS_ANDROID)
#include "cpp/tools/AndroidService.h"
#include "private/qandroidextras_p.h" // for QAndroidService
#endif

#if QT_CONFIG(permissions)
#include <QPermissions>
#endif

/* ************************************************************************** */






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









int main(int argc, char* argv[])
{

//QQuickStyle::setStyle("Material");

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
          // NVIDIA driver suspend&resume hack
    auto format = QSurfaceFormat::defaultFormat();
    format.setOption(QSurfaceFormat::ResetNotification);
    QSurfaceFormat::setDefaultFormat(format);
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    // Qt 6.6+ mouse wheel hack
    qputenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", "2500");
#endif

#if defined(Q_OS_ANDROID) && (QT_VERSION <= QT_VERSION_CHECK(6,6,1))
    // Force "old" Android native multimedia backend
    // android backend doesn't work past Qt 6.6.1
    // ffmpeg backend doesn't work below Qt 6.6.2
    // (ffmpeg multimedia backend is buggy as hell...)
  //  qputenv("QT_MEDIA_BACKEND", "android");
#endif

    //qputenv("QT_MEDIA_BACKEND", "ffmpeg");
    //qputenv("QT_MEDIA_BACKEND", "android");
    //qputenv("QT_MEDIA_BACKEND", "gstreamer");

    // GUI application /////////////////////////////////////////////////////////

/*#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    SharingApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
    QIcon appIcon(":/myassets/gfx/logos/DigitalBloom.svg");
    app.setWindowIcon(appIcon);
#endif */
    // Arguments parsing ///////////////////////////////////////////////////////

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


#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_LINUX)

#else

#endif


    qputenv("QT_QUICK_CONTROLS_STYLE", "material");
    defaultssl();

    registerTypes();


    /* QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "*.warning=false\n"
                                     "*.info=false"); */




#ifdef Q_OS_ANDROID
   // QZXing::registerQMLTypes();
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

    // GUI application /////////////////////////////////////////////////////////

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    ShareUtils *utilsShare = new ShareUtils();
#else
    //  QIcon appIcon(":/assets/gfx/logos/DigitalBloom.svg");
    //  app.setWindowIcon(appIcon);

    SystrayManager *st = SystrayManager::getInstance();
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_LINUX)
  SharingApplication app(argc, argv);

#else
    SingleApplication app(argc, argv, true);
#endif


    app.setApplicationName("DigitalBloom");
    app.setOrganizationName("stllimited");
    app.setOrganizationDomain("stllimited");

    // Init app components
    SettingsManager *sm = SettingsManager::getInstance();
    DatabaseManager *db = DatabaseManager::getInstance();
    NotificationManager *nm = NotificationManager::getInstance();
    if (!sm || !db || !nm)
    {
        qWarning() << "Cannot init app components!";
        return EXIT_FAILURE;
    }


    UtilsLanguage *utilsLanguage = UtilsLanguage::getInstance();



    // Sun and Moon
  /*  SunAndMoon sam;
    if (sm->getSunAndMoon())
    {
        sam.set(sm->getLatitude(), sm->getLongitude(), QDateTime::currentDateTime());
    } */

    // Init generic utils
    // Init app components
    DatabaseManager *dbm = DatabaseManager::getInstance();
    if (!dbm) return EXIT_FAILURE;

    SettingsManager *stm = SettingsManager::getInstance();
    if (!stm) return EXIT_FAILURE;

    BarcodeManager *bch = BarcodeManager::getInstance();
    if (!bch) return EXIT_FAILURE;

    // Init app utils
    UtilsApp *utilsApp = UtilsApp::getInstance();
    if (!utilsApp) return EXIT_FAILURE;

    UtilsScreen *utilsScreen = UtilsScreen::getInstance();
    if (!utilsScreen) return EXIT_FAILURE;

    UtilsClipboard *utilsClipboard = new UtilsClipboard();
    if (!utilsClipboard) return EXIT_FAILURE;

    UtilsCamera *utilsCamera = UtilsCamera::getInstance();
    if (!utilsCamera) return EXIT_FAILURE;

    bool qtConnectivityPatched = false;
#if defined(QT_CONNECTIVITY_PATCHED)
    qtConnectivityPatched = true;
#endif

MobileUI::registerQML();

  //  qmlRegisterSingletonType(QUrl("qrc:/qml/ThemeEngine.qml"), "ThemeEngine", 1, 0, "EricTheme");
    ericassets.setenvssl();

    QQmlApplicationEngine engine;






/*#if defined(Q_OS_LINUX) || defined(Q_OS_WIN64)
    QZXing::registerQMLTypes();
    QZXing::registerQMLImageProvider(engine);
#endif */


    EricType mytype;
    MarkerModel model;
    QQmlContext* context = engine.rootContext();
  //  context->setContextProperty("sunAndMoon", &sam);

    context->setContextProperty("settingsManager", stm);
    context->setContextProperty("barcodeManager", bch);
    context->setContextProperty("utilsApp", utilsApp);
    context->setContextProperty("utilsScreen", utilsScreen);
    context->setContextProperty("utilsCamera", utilsCamera);
    context->setContextProperty("utilsClipboard", utilsClipboard);

  //  context->setContextProperty("startMinimized", (start_minimized || sm->getMinimized()));
  //  context->setContextProperty("qtConnectivityPatched", qtConnectivityPatched);
    context->setContextProperty("markerModel", &model);
    context->setContextProperty("settingsManager", sm);
   // context->setContextProperty("myutilsApp", utilsApp);
  //  context->setContextProperty("myutilsLanguage", utilsLanguage);
   // context->setContextProperty("myutilsScreen", utilsScreen);
    mytype.addContextProperty(context);
   // app.registerQML(context);

#if QT_CONFIG(permissions)
    if (qApp->checkPermission(QCameraPermission{}) != Qt::PermissionStatus::Granted) {
        qApp->requestPermission(QCameraPermission{}, [](const QPermission &permission) {
            if (permission.status() != Qt::PermissionStatus::Granted) {
                qWarning() << "Impossible to get Camera permission!";
            }
        });
    }
#endif


#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)

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

    // ──── REGISTER APPLICATION ────





    qmlRegisterType<StatusBar>("StatusBar", 0, 1, "StatusBar");


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


    // Load Qaterial
   // qaterial::loadQmlResources();
   // qaterial::registerQmlTypes();



    // ──── LOAD QML MAIN ────



    // Load the main view
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(FORCE_MOBILE_UI)
    context->setContextProperty("utilsShare", utilsShare);
    /*  const QUrl url(QStringLiteral("qrc:/qml/ui/MobileApplication.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url); */
    engine.loadFromModule("DigitalBloom", "MobileApplication");
#else
    context->setContextProperty("systrayManager", st);
    /*  const QUrl url(QStringLiteral("qrc:/qml/ui/DesktopApplication.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);*/
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
    context->setContextProperty("quickWindow", window);
    FrameRateMonitor *utilsFpsMonitor = new FrameRateMonitor(window);
    context->setContextProperty("utilsFpsMonitor", utilsFpsMonitor);

    // React to secondary instances
   QObject::connect(&app, &SingleApplication::instanceStarted, window, &QQuickWindow::show);
   QObject::connect(&app, &SingleApplication::instanceStarted, window, &QQuickWindow::raise);

    // Systray?
  //  st->setupSystray(window);
   // if (sm->getSysTray()) st->installSystray();


#if defined(Q_OS_MACOS)
    // macOS dock
    MacOSDockHandler *dockIconHandler = MacOSDockHandler::getInstance();
    dockIconHandler->setupDock(window);
    engine_context->setContextProperty("utilsDock", dockIconHandler);
#endif

#endif // desktop section

#if defined(Q_OS_ANDROID)
    QNativeInterface::QAndroidApplication::hideSplashScreen(333);
  //  if (sm->getSysTray()) AndroidService::service_start();
#endif

    return app.exec();
}
