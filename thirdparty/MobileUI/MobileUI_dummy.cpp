#include "MobileUI_private.h"

/* ************************************************************************** */

int MobileUIPrivate::getDeviceTheme()
{
    return MobileUI::Theme::Light;
}

void MobileUIPrivate::setColor_statusbar(const QColor &color)
{
    Q_UNUSED(color)
}

void MobileUIPrivate::setTheme_statusbar(const MobileUI::Theme theme)
{
    Q_UNUSED(theme)
}

void MobileUIPrivate::setColor_navbar(const QColor &color)
{
    Q_UNUSED(color)
}

void MobileUIPrivate::setTheme_navbar(const MobileUI::Theme theme)
{
    Q_UNUSED(theme)
}

int MobileUIPrivate::getStatusbarHeight()
{
    return 0;
}

int MobileUIPrivate::getNavbarHeight()
{
    return 0;
}

int MobileUIPrivate::getSafeAreaTop()
{
    return 0;
}

int MobileUIPrivate::getSafeAreaLeft()
{
    return 0;
}

int MobileUIPrivate::getSafeAreaRight()
{
    return 0;
}

int MobileUIPrivate::getSafeAreaBottom()
{
    return 0;
}

void MobileUIPrivate::setScreenAlwaysOn(const bool on)
{
    Q_UNUSED(on)
}

void MobileUIPrivate::setScreenOrientation(const MobileUI::ScreenOrientation orientation)
{
    Q_UNUSED(orientation)
}

int MobileUIPrivate::getScreenBrightness()
{
    return -1;
}

void MobileUIPrivate::setScreenBrightness(const int value)
{
    Q_UNUSED(value)
}

void MobileUIPrivate::vibrate()
{
    return;
}

void MobileUIPrivate::backToHomeScreen()
{
    return;
}

/* ************************************************************************** */
