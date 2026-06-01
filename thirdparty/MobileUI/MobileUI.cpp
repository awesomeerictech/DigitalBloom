#include "MobileUI.h"
#include "MobileUI_private.h"

#include <QGuiApplication>
#include <QQmlEngine>
#include <QScreen>

/* ************************************************************************** */

bool MobileUI::isPhone = false;
bool MobileUI::isTablet = false;

bool MobileUIPrivate::areRefreshSlotsConnected = false;

MobileUI::Theme MobileUIPrivate::deviceTheme = MobileUI::Light;

QColor MobileUIPrivate::statusbarColor;
MobileUI::Theme MobileUIPrivate::statusbarTheme = MobileUI::Light;

QColor MobileUIPrivate::navbarColor;
MobileUI::Theme MobileUIPrivate::navbarTheme = MobileUI::Light;

bool MobileUIPrivate::screenAlwaysOn = false;

MobileUI::ScreenOrientation MobileUIPrivate::screenOrientation = MobileUI::Unlocked;

/* ************************************************************************** */

void MobileUI::registerQML()
{
    qRegisterMetaType<MobileUI::Theme>("MobileUI::Theme");
    qRegisterMetaType<MobileUI::ScreenOrientation>("MobileUI::ScreenOrientation");

    qmlRegisterType<MobileUI>("MobileUI", 1, 0, "MobileUI");
}

/* ************************************************************************** */

MobileUI::MobileUI(QObject *parent) : QObject(parent)
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    QScreen *screen = qApp->primaryScreen();
    if (screen)
    {
        double screenSizeInch = std::sqrt(std::pow(screen->physicalSize().width(), 2.0) +
                                          std::pow(screen->physicalSize().height(), 2.0)) / (2.54 * 10.0);

        if (screenSizeInch < 7.0) MobileUI::isPhone = true;
        else MobileUI::isTablet = true;
    }
#endif
}

/* ************************************************************************** */

MobileUI::Theme MobileUI::getDeviceTheme()
{
    return static_cast<MobileUI::Theme>(MobileUIPrivate::getDeviceTheme());
}

/* ************************************************************************** */

QColor MobileUI::getStatusbarColor()
{
    return MobileUIPrivate::statusbarColor;
}

void MobileUI::setStatusbarColor(const QColor &color)
{
    if (color.isValid())
    {
        MobileUIPrivate::statusbarColor = color;
        MobileUIPrivate::setColor_statusbar(color);
    }
}

MobileUI::Theme MobileUI::getStatusbarTheme()
{
    return MobileUIPrivate::statusbarTheme;
}

void MobileUI::setStatusbarTheme(const MobileUI::Theme theme)
{
    MobileUIPrivate::statusbarTheme = theme;
    MobileUIPrivate::setTheme_statusbar(theme);
}

/* ************************************************************************** */

QColor MobileUI::getNavbarColor()
{
    return MobileUIPrivate::navbarColor;
}

void MobileUI::setNavbarColor(const QColor &color)
{
    if (color.isValid())
    {
        MobileUIPrivate::navbarColor = color;
        MobileUIPrivate::setColor_navbar(color);
    }
}

MobileUI::Theme MobileUI::getNavbarTheme()
{
    return MobileUIPrivate::navbarTheme;
}

void MobileUI::setNavbarTheme(const MobileUI::Theme theme)
{
    MobileUIPrivate::navbarTheme = theme;
    MobileUIPrivate::setTheme_navbar(theme);
}

/* ************************************************************************** */

void MobileUI::refreshUI()
{
    if (MobileUIPrivate::statusbarColor.isValid())
        MobileUIPrivate::setColor_statusbar(MobileUIPrivate::statusbarColor);

    if (MobileUIPrivate::navbarColor.isValid())
        MobileUIPrivate::setColor_navbar(MobileUIPrivate::navbarColor);

    MobileUIPrivate::setTheme_statusbar(MobileUIPrivate::statusbarTheme);
    MobileUIPrivate::setTheme_navbar(MobileUIPrivate::navbarTheme);
}

/* ************************************************************************** */

int MobileUI::getStatusbarHeight()
{
    return MobileUIPrivate::getStatusbarHeight();
}

int MobileUI::getNavbarHeight()
{
    return MobileUIPrivate::getNavbarHeight();
}

int MobileUI::getSafeAreaTop()
{
    return MobileUIPrivate::getSafeAreaTop();
}

int MobileUI::getSafeAreaLeft()
{
    return MobileUIPrivate::getSafeAreaLeft();
}

int MobileUI::getSafeAreaRight()
{
    return MobileUIPrivate::getSafeAreaRight();
}

int MobileUI::getSafeAreaBottom()
{
    return MobileUIPrivate::getSafeAreaBottom();
}

/* ************************************************************************** */

MobileUI::ScreenOrientation MobileUI::getScreenOrientation()
{
    return MobileUIPrivate::screenOrientation;
}

void MobileUI::setScreenOrientation(const MobileUI::ScreenOrientation orientation)
{
    MobileUIPrivate::screenOrientation = orientation;
    MobileUIPrivate::setScreenOrientation(orientation);
}

bool MobileUI::getScreenAlwaysOn()
{
    return MobileUIPrivate::screenAlwaysOn;
}

void MobileUI::setScreenAlwaysOn(const bool value)
{
    MobileUIPrivate::screenAlwaysOn = value;
    MobileUIPrivate::setScreenAlwaysOn(value);
}

/* ************************************************************************** */

int MobileUI::getScreenBrightness()
{
    return MobileUIPrivate::getScreenBrightness();
}

void MobileUI::setScreenBrightness(const int value)
{
    return MobileUIPrivate::setScreenBrightness(value);
}

/* ************************************************************************** */

void MobileUI::vibrate()
{
    MobileUIPrivate::vibrate();
}

void MobileUI::backToHomeScreen()
{
    MobileUIPrivate::backToHomeScreen();
}

/* ************************************************************************** */
