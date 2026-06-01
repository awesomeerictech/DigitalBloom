#include "utils_os_windows.h"

#if defined(Q_OS_WINDOWS)

#include <windows.h>
#include <QDebug>

/* ************************************************************************** */

void UtilsWindows::screenKeepOn(bool on)
{
    // ES_DISPLAY_REQUIRED prevents display sleep?
    // ES_SYSTEM_REQUIRED prevents idle sleep

    EXECUTION_STATE result;
    if (on)
    {
        result = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
    }
    else
    {
        result = SetThreadExecutionState(ES_CONTINUOUS);
    }

    if (result == NULL)
    {
        qWarning() << "screenKeepOn() failed";
    }
}

/* ************************************************************************** */
#endif // Q_OS_WINDOWS
