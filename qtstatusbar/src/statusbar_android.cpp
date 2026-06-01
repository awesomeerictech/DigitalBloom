
#include "statusbar_p.h"

#include <QtCore>
#include <QCoreApplication>
#define FLAG_TRANSLUCENT_STATUS 0x04000000
#define FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS 0x80000000
// View
#define SYSTEM_UI_FLAG_LIGHT_STATUS_BAR 0x00002000



bool StatusBarPrivate::isAvailable_sys()
{
    return QNativeInterface::QAndroidApplication::sdkVersion() >= 21;
}

void StatusBarPrivate::setColor_sys(const QColor &color)
{
    if (QNativeInterface::QAndroidApplication::sdkVersion() < 21)
        return;


    auto task = QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {

        QJniObject window = QNativeInterface::QAndroidApplication::context();

        return QVariant::fromValue(window);
    }).then([=](QFuture<QVariant> future) {
        auto window = future.result().value<QJniObject>();
        if (window.isValid()) {

            QJniObject mywin =  window.callObjectMethod("getWindow", "()Landroid/view/Window;");
            mywin.callMethod<void>("addFlags", "(I)V", FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            mywin.callMethod<void>("clearFlags", "(I)V", FLAG_TRANSLUCENT_STATUS);
            mywin.callMethod<void>("setStatusBarColor", "(I)V", color.rgba());
            qDebug() << "Retrieved window object is valid";

                    }
    });
}

void StatusBarPrivate::setTheme_sys(StatusBar::Theme theme)
{
    if (QNativeInterface::QAndroidApplication::sdkVersion() < 23)
        return;

    auto task = QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {

        QJniObject window = QNativeInterface::QAndroidApplication::context();

        return QVariant::fromValue(window);
    }).then([=](QFuture<QVariant> future) {
        auto window = future.result().value<QJniObject>();
        if (window.isValid()) {


            QJniObject mywindow =  window.callObjectMethod("getWindow", "()Landroid/view/Window;");
            QJniObject view = mywindow.callObjectMethod("getDecorView", "()Landroid/view/View;");
            int visibility = view.callMethod<int>("getSystemUiVisibility", "()I");
            if (theme == StatusBar::Theme::Light)
                visibility |= SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
            else
                visibility &= ~SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
            view.callMethod<void>("setSystemUiVisibility", "(I)V", visibility);


                    }
    });
}
