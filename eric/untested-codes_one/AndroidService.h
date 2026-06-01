// AndroidService.h
#pragma once

#include <QString>

class AndroidService {
public:
    // serverId: 0=StreamServer (zlm), 1=Qx controller, 2=Wt controller
    static void serviceStart(int serverId, const QString &iniPath = QString(), const QString &sslPath = QString());
    static void serviceStop(int serverId);
};
