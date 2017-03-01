/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#include "config.h"

const QString Config::logFileName = "launcher-log.txt";

const QString Config::dataFileName = "launcher.dat";
const int Config::dataResourceId = 3151;
const int Config::dataResourceTypeId = 10;

const QString Config::patcherDirectoryName = "patcher";
const QString Config::patcherInstallationInfoFileName = "install_info";
const QString Config::patcherVersionInfoFileName = "version_info";
const QString Config::patcherIdInfoFileName = "id_info";
const QString Config::patcherManifestFileName = "patcher.manifest";

const QString Config::applicationDirectoryName = "app";

const int Config::minConnectionTimeoutMsec = 10000;
const int Config::maxConnectionTimeoutMsec = 30000;

const int Config::chunkedDownloadStaleTimeoutMsec = 120000;

const int Config::timeBetweenContentUrlsIterations = 10000;

const QString Config::mainApiUrl = "http://api.patchkit.net";
const QStringList Config::cacheApiUrls = (QStringList() << "http://api-cache-1.patchkit.net"
    << "http://api-cache-2.patchkit.net"
    << "http://api-cache-3.patchkit.net");

const QString Config::pingTarget = "8.8.8.8";

#if defined(_WIN64) || defined(_WIN32)
const QString Config::pingCountArg = "-n";
#else
const QString Config::pingCountArg = "-c";
#endif
