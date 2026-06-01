#ifndef MOBILEUI_PRIVATE_H
#define MOBILEUI_PRIVATE_H
/* ************************************************************************** */

#include "MobileUI.h"

/* ************************************************************************** */

class MobileUIPrivate
{
public:
    static bool areRefreshSlotsConnected;

    static MobileUI::Theme deviceTheme;

    static QColor statusbarColor;
    static MobileUI::Theme statusbarTheme;

    static QColor navbarColor;
    static MobileUI::Theme navbarTheme;

    static bool screenAlwaysOn;

    static MobileUI::ScreenOrientation screenOrientation;

    static int getDeviceTheme();

    static void refreshUI_async();

    static void setColor_statusbar(const QColor &color);
    static void setTheme_statusbar(const MobileUI::Theme theme);

    static void setColor_navbar(const QColor &color);
    static void setTheme_navbar(const MobileUI::Theme theme);

    static int getStatusbarHeight();
    static int getNavbarHeight();

    static int getSafeAreaTop();
    static int getSafeAreaLeft();
    static int getSafeAreaRight();
    static int getSafeAreaBottom();

    static void setScreenAlwaysOn(const bool on);

    static void setScreenOrientation(const MobileUI::ScreenOrientation orientation);

    static int getScreenBrightness();
    static void setScreenBrightness(const int value);

    static void vibrate();

    static void backToHomeScreen();
};

/* ************************************************************************** */
#endif // MOBILEUI_PRIVATE_H
