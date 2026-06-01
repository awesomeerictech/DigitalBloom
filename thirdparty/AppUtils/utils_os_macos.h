#ifndef UTILS_MACOS_H
#define UTILS_MACOS_H

#include <QtGlobal>

#if defined(Q_OS_MACOS)
/* ************************************************************************** */

#include <QString>

/*!
 * \brief macOS utils
 *
 * Use with "LIBS += -framework IOKit"
 *
 * Registering and unregistering for sleep and wake notifications:
 * - https://developer.apple.com/library/archive/qa/qa1340/_index.html
 */
class UtilsMacOS
{
public:
    static uint32_t screenKeepOn(const QString &application, const QString &reason);
    static void screenKeepAuto(uint32_t screensaverId);
};

/* ************************************************************************** */
#endif // Q_OS_MACOS
#endif // UTILS_MACOS_H
