

#include "statusbar_p.h"

bool StatusBarPrivate::isAvailable_sys()
{
    return false;
}

void StatusBarPrivate::setColor_sys(const QColor &color)
{
    Q_UNUSED(color);
}

void StatusBarPrivate::setTheme_sys(StatusBar::Theme theme)
{
    Q_UNUSED(theme);
}
