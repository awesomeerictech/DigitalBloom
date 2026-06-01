
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QObject>
#include <QColor>

class StatusBar : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ isAvailable CONSTANT)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(Theme theme READ theme WRITE setTheme)

public:
    explicit StatusBar(QObject *parent = nullptr);

    static bool isAvailable();

    static QColor color();
    static void setColor(const QColor &color);

    enum Theme { Light, Dark };
    Q_ENUM(Theme)

    static Theme theme();
    static void setTheme(Theme theme);
};

#endif // STATUSBAR_H
