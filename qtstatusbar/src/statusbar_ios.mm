
#include "statusbar_p.h"

#include <UIKit/UIKit.h>
#include <QGuiApplication>

#include <QScreen>
#include <QTimer>

@interface QIOSViewController : UIViewController
@property (nonatomic, assign) BOOL prefersStatusBarHidden;
@property (nonatomic, assign) UIStatusBarAnimation preferredStatusBarUpdateAnimation;
@property (nonatomic, assign) UIStatusBarStyle preferredStatusBarStyle;
@end

#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)

bool StatusBarPrivate::isAvailable_sys()
{
    return true;
}

void StatusBarPrivate::setColor_sys(const QColor &color)
{
    Q_UNUSED(color);
}

static UIStatusBarStyle statusBarStyle(StatusBar::Theme theme)
{
    if (theme == StatusBar::Dark)
        return UIStatusBarStyleLightContent;
    else if (SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(@"13.0"))
        return UIStatusBarStyleDarkContent;
    else
        return UIStatusBarStyleDefault;
}

static void setPreferredStatusBarStyle(UIWindow *window, UIStatusBarStyle style)
{
    QIOSViewController *viewController = static_cast<QIOSViewController *>([window rootViewController]);
    if (!viewController || viewController.preferredStatusBarStyle == style)
        return;

    viewController.preferredStatusBarStyle = style;
    [viewController setNeedsStatusBarAppearanceUpdate];
}

void togglePreferredStatusBarStyle()
{
    UIStatusBarStyle style = statusBarStyle(StatusBar::Light);
    if(StatusBarPrivate::theme == StatusBar::Light) {
        style = statusBarStyle(StatusBar::Dark);
    }
    UIWindow *keyWindow = [[UIApplication sharedApplication] keyWindow];
    if (keyWindow)
        setPreferredStatusBarStyle(keyWindow, style);
    QTimer::singleShot(200, []() {
        UIStatusBarStyle style = statusBarStyle(StatusBarPrivate::theme);
        UIWindow *keyWindow = [[UIApplication sharedApplication] keyWindow];
        if (keyWindow)
            setPreferredStatusBarStyle(keyWindow, style);
    });
}

static void updatePreferredStatusBarStyle()
{
    UIStatusBarStyle style = statusBarStyle(StatusBarPrivate::theme);
    UIWindow *keyWindow = [[UIApplication sharedApplication] keyWindow];
    if (keyWindow)
        setPreferredStatusBarStyle(keyWindow, style);
}

void StatusBarPrivate::setTheme_sys(StatusBar::Theme)
{
    updatePreferredStatusBarStyle();

    QObject::connect(qApp, &QGuiApplication::applicationStateChanged, qApp, [](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive)
            updatePreferredStatusBarStyle();
    }, Qt::UniqueConnection);

    QScreen *screen = qApp->primaryScreen();
    screen->setOrientationUpdateMask(Qt::PortraitOrientation | Qt::LandscapeOrientation | Qt::InvertedPortraitOrientation | Qt::InvertedLandscapeOrientation);
    QObject::connect(screen, &QScreen::orientationChanged, qApp, [](Qt::ScreenOrientation) {
        togglePreferredStatusBarStyle();
    }, Qt::UniqueConnection);
}
