// start stream server (zlm) id 0
QString ini = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/zlmediakit.ini";
QString ssl = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/ssl.pem";
AndroidService::serviceStart(0, ini, ssl);

// start qx controller (id 1)
AndroidService::serviceStart(1);

// start wt controller (id 2)
AndroidService::serviceStart(2);

// stop:
AndroidService::serviceStop(1);
AndroidService::serviceStop(2);
AndroidService::serviceStop(0);
