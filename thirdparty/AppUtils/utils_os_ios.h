#ifndef UTILS_OS_IOS_H
#define UTILS_OS_IOS_H

#include <QtGlobal>
#include <QString>

#if defined(Q_OS_IOS)
/* ************************************************************************** */

/*!
 * \brief iOS utils
 */
class UtilsIOS
{
public:
    static void screenKeepOn(bool on);

    static void screenLockOrientation(int orientation);

    static void screenLockOrientation(int orientation, bool autoRotate);

    static void vibrate(int milliseconds);
};

/* ************************************************************************** */
#endif // Q_OS_IOS
#endif // UTILS_OS_IOS_H
