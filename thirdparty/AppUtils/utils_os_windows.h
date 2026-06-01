#ifndef UTILS_OS_WINDOWS_H
#define UTILS_OS_WINDOWS_H

#include <QtGlobal>

#if defined(Q_OS_WINDOWS)
/* ************************************************************************** */

class UtilsWindows
{
public:
    /*!
     * \param on
     */
   static void screenKeepOn(bool on);
};

/* ************************************************************************** */
#endif // Q_OS_WINDOWS
#endif // UTILS_OS_WINDOWS_H
