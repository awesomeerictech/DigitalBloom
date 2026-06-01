#ifndef UTILS_OS_LINUX_H
#define UTILS_OS_LINUX_H

#include <QtGlobal>

#if defined(Q_OS_LINUX)
/* ************************************************************************** */

#include <QString>

/*!
 * \brief Linux utils
 *
 * Use with "QT += dbus"
 */
class UtilsLinux
{
public:
   static uint32_t screenKeepOn(const QString &application, const QString &reason);
   static void screenKeepAuto(uint32_t screensaverId);
};

/* ************************************************************************** */
#endif // Q_OS_LINUX
#endif // UTILS_OS_LINUX_H
