/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#pragma once

#include <QStringList>

class Config
{
public:
    const static QString logFileName;

    const static QString dataFileName;
    const static int dataResourceId;
    const static int dataResourceTypeId;

    const static QString patcherDirectoryName;
    const static QString patcherInstallationInfoFileName;
    const static QString patcherVersionInfoFileName;
    const static QString patcherIdInfoFileName;
    const static QString patcherManifestFileName;

    const static QString applicationDirectoryName;

    const static int minConnectionTimeoutMsec;
    const static int maxConnectionTimeoutMsec;

    const static int chunkedDownloadStaleTimeoutMsec;

    const static int timeBetweenContentUrlsIterations;

    const static QString mainApiUrl;
    const static QStringList cacheApiUrls;

    const static QString pingTarget;
    const static QString pingCountArg;
};
