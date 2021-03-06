/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#pragma once

#include "data.h"
#include "remotepatcherdata.h"
#include "localpatcherdata.h"
#include "cancellationtokensource.h"
#include "api.h"

class LauncherWorker : public QThread
{
    Q_OBJECT

    void run() override;

public:
    enum Result
    {
        NONE,
        CANCELLED,
        SUCCESS,
        FAILED,
        FATAL_ERROR
    };

    LauncherWorker();

    void cancel();

    Result result() const;
signals:
    void statusChanged(const QString& t_status);
    void progressChanged(int t_progress);

private slots:
    void setDownloadProgress(const long long& t_bytesDownloaded, const long long& t_totalBytes);

private:
#ifdef Q_OS_WIN
    void runWithDataFromResource();
#endif
    void runWithDataFromFile();

    void runWithData(Data& t_data);

    void setupPatcherSecret(Data& t_data);

    bool tryToFetchPatcherSecret(Data& t_data);

    void updatePatcher(const Data& t_data);
    void startPatcher(const Data& t_data);

    void checkIfCurrentDirectoryIsWritable();

    Api m_api;
    RemotePatcherData m_remotePatcher;
    LocalPatcherData m_localPatcher;

    std::shared_ptr<CancellationTokenSource> m_cancellationTokenSource;

    Result m_result;

    QNetworkAccessManager m_networkAccessManager;
};
