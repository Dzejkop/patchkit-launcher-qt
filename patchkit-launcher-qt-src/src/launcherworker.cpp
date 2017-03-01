/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#include "launcherworker.h"

#include <QtMath>
#include <QMessageBox>

#include "logger.h"
#include "locations.h"
#include "fatalexception.h"
#include "downloader.h"

#if defined(Q_OS_WIN)
#include <Windows.h>
#endif

void LauncherWorker::run()
{
    try
    {
#ifdef Q_OS_WIN
        try
        {
            runWithDataFromResource();
            return;
        }
        catch (std::exception& exception)
        {
            logWarning(exception.what());
        }

        logWarning("Running with data from resources has failed. Trying to run with data from file located in current directory.");
#endif

        runWithDataFromFile();

        m_result = SUCCESS;
        logInfo("Launcher has succeeded.");
    }
    catch (CancelledException&)
    {
        m_result = CANCELLED;
        logInfo("Launcher has been canceled.");
    }
    catch (FatalException& exception)
    {
        m_result = FATAL_ERROR;
        logCritical(exception.what());
    }
    catch (std::exception& exception)
    {
        m_result = FAILED;
        logWarning(exception.what());
    }
    catch (...)
    {
        m_result = FATAL_ERROR;
        logCritical("Unknown exception.");
    }
}

LauncherWorker::LauncherWorker()
    : m_cancellationTokenSource(new CancellationTokenSource())
    , m_result(NONE)
    , m_remotePatcher(m_api, &m_networkAccessManager)
{
    m_api.moveToThread(this);
    m_networkAccessManager.moveToThread(this);
    m_remotePatcher.moveToThread(this);
    m_localPatcher.moveToThread(this);
}

void LauncherWorker::cancel()
{
    logInfo("Cancelling launcher thread.");

    m_cancellationTokenSource->cancel();
}

LauncherWorker::Result LauncherWorker::result() const
{
    return m_result;
}

void LauncherWorker::setDownloadProgress(const long long& t_bytesDownloaded, const long long& t_totalBytes)
{
    long long kilobytesDownloaded = t_bytesDownloaded / 1024;
    long long totalKilobytes = t_totalBytes / 1024;

    emit statusChanged(QString("Downloading %1 / %2 KB").arg(QString::number(kilobytesDownloaded), QString::number(totalKilobytes)));
    emit progressChanged(qCeil((qreal(t_bytesDownloaded) / t_totalBytes) * 100.0));
}

#ifdef Q_OS_WIN

void LauncherWorker::runWithDataFromResource()
{
    logInfo("Starting launcher with data from resource.");

    Data data = Data::loadFromResources(Locations::getInstance().applicationFilePath(),
                                        Config::dataResourceId,
                                        Config::dataResourceTypeId);

    runWithData(data);
}

#endif

void LauncherWorker::runWithDataFromFile()
{
    logInfo("Starting launcher with data from file.");

    Data data = Data::loadFromFile(Locations::getInstance().dataFilePath());

    runWithData(data);
}

void LauncherWorker::runWithData(Data& t_data)
{
    try
    {
        logInfo("Starting launcher.");

        setupPatcherSecret(t_data);

        logInfo("Current directory set to - %1", .arg(Locations::getInstance().currentDirPath()));

        updatePatcher(t_data);
    }
    catch (CancelledException&)
    {
        throw;
    }
    catch (FatalException&)
    {
        throw;
    }
    catch (std::exception& exception)
    {
        if (m_localPatcher.isInstalled())
        {
            logWarning(exception.what());
            logWarning("Updating patcher failed but previous patcher is still available.");
        }
        else
        {
            throw;
        }
    }
    catch (...)
    {
        if (m_localPatcher.isInstalled())
        {
            logWarning("Unknown exception.");
            logWarning("Updating patcher failed but previous patcher is still available.");
        }
        else
        {
            throw;
        }
    }

    startPatcher(t_data);
}

void LauncherWorker::setupPatcherSecret(Data& t_data)
{
    QSettings settings("UpSoft", t_data.applicationSecret().append("launcher-"));

    if (tryToFetchPatcherSecret(t_data))
    {
        settings.setValue("patcher_secret", t_data.overwritePatcherSecret);
    }
    else if (settings.contains("patcher_secret"))
    {
        t_data.overwritePatcherSecret = settings.value("patcher_secret").toString();
    }
}

bool LauncherWorker::tryToFetchPatcherSecret(Data& t_data)
{
    try
    {
        t_data.overwritePatcherSecret = m_remotePatcher.getPatcherSecret(t_data, m_cancellationTokenSource);
        return true;
    }
    catch (CancelledException&)
    {
        throw;
    }
    catch (FatalException&)
    {
        throw;
    }
    catch (std::exception& exception)
    {
        logWarning(exception.what());
        return false;
    }
    catch (...)
    {
        logWarning("Unknown exception while fetching patcher secret.");
        return false;
    }
}

void LauncherWorker::updatePatcher(const Data& t_data)
{
    logInfo("Updating patcher.");

    int version = m_remotePatcher.getVersion(t_data, m_cancellationTokenSource);
    logDebug("Current remote patcher version - %1", .arg(QString::number(version)));

    emit progressChanged(0);
    emit statusChanged("Waiting...");

    if (!m_localPatcher.isInstalledSpecific(version, t_data))
    {
        logInfo("Checking whether current directory is writable.");

        checkIfCurrentDirectoryIsWritable();

        logInfo("The newest patcher is not installed. Downloading the newest version of patcher.");

        emit statusChanged("Downloading...");

        logDebug("Connecting downloadProgressChanged signal from remote patcher to slot from launcher thread.");
        connect(&m_remotePatcher, &RemotePatcherData::downloadProgressChanged, this, &LauncherWorker::setDownloadProgress);

        QString downloadPath = QDir::cleanPath(Locations::getInstance().applicationDirPath() + "/patcher.zip");

        QFile file(downloadPath);

        m_remotePatcher.download(file, t_data, version, m_cancellationTokenSource);
        logInfo("Patcher has been downloaded to %1", .arg(downloadPath));

        logDebug("Disconnecting downloadProgressChanged signal from remote patcher to slot from launcher thread.");
        disconnect(&m_remotePatcher, &RemotePatcherData::downloadProgressChanged, this, &LauncherWorker::setDownloadProgress);

        emit progressChanged(100);
        emit statusChanged("Installing...");

        m_localPatcher.install(downloadPath, t_data, version);

        QFile::remove(downloadPath);
        logInfo("Patcher has been installed.");
    }
}

void LauncherWorker::startPatcher(const Data& t_data)
{
    logInfo("Starting patcher.");

    emit statusChanged("Starting...");

    m_localPatcher.start(t_data);
}

void LauncherWorker::checkIfCurrentDirectoryIsWritable()
{
    if (!Locations::getInstance().isCurrentDirWritable())
    {
#if defined(Q_OS_WIN)
        ShellExecute(nullptr, L"runas", Locations::getInstance().applicationFilePath().toStdWString().c_str(), nullptr, nullptr, SW_SHOWNORMAL);

        throw CancelledException();
#else
        throw FatalException("Current application directory isn't writable.");
#endif
    }
}
