/*!
 * DigitalBloom C++ Service Controller
 * This file contains the C++ functions to control the DigitalBloomService
 * via JNI, using the static helper methods defined in the Java service.
 */

#include <QtGlobal>
#include <QObject>

#if defined(Q_OS_ANDROID)

#include <QtCore/private/qandroidextras_p.h>
#include <QDebug>


// --- Static Control Functions (To be called from C++ code) ---

void DigitalBloomService_start()
{
    qDebug() << "DigitalBloomServiceController: Requesting Java service start...";
    // Call the static Java method: org.stlltd.digitalbloom.DigitalBloomService.serviceStart(Context)
    QJniObject::callStaticMethod<void>("org/stlltd/digitalbloom/DigitalBloomService",
                                       "serviceStart",
                                       "(Landroid/content/Context;)V",
                                       QNativeInterface::QAndroidApplication::context());
}

void DigitalBloomService_stop()
{
    qDebug() << "DigitalBloomServiceController: Requesting Java service stop...";
    // Call the static Java method: org.stlltd.digitalbloom.DigitalBloomService.serviceStop(Context)
    QJniObject::callStaticMethod<void>("org/stlltd/digitalbloom/DigitalBloomService",
                                       "serviceStop", 
                                       "(Landroid/content/Context;)V",
                                       QNativeInterface::QAndroidApplication::context());
}



#endif // Q_OS_ANDROID