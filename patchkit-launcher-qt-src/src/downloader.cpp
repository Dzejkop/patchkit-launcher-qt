/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#include "downloader.h"

#include "logger.h"
#include "timeoutexception.h"
#include "config.h"

Downloader::Downloader(QNetworkAccessManager* t_dataSource, CancellationToken& t_cancellationToken)
    : m_remoteDataSource(t_dataSource)
    , m_cancellationToken(t_cancellationToken)
{
}

QByteArray Downloader::downloadFile(const QString& t_urlPath, int t_requestTimeoutMsec, int* t_replyStatusCode)
{
    QNetworkRequest request(t_urlPath);

    return downloadFile(request, t_requestTimeoutMsec, t_replyStatusCode);
}

QByteArray Downloader::downloadFile(const QNetworkRequest& t_request, int t_requestTimeoutMsec, int* t_replyStatusCode)
{
    TRemoteDataReply reply;

    fetchReply(t_request, reply);

    connect(reply.data(), &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgressChanged);

    waitForReply(reply, t_requestTimeoutMsec);
    validateReply(reply);

    int replyStatusCode = getReplyStatusCode(reply);

    if (t_replyStatusCode != nullptr)
    {
        *t_replyStatusCode = replyStatusCode;
    }

    if (replyStatusCode < 200 || replyStatusCode >= 300)
    {
        return QByteArray();
    }

    waitForFileDownload(reply);

    disconnect(reply.data(), &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgressChanged);

    return reply->readAll();
}

QString Downloader::downloadString(const QString& t_urlPath, int t_requestTimeoutMsec, int& t_replyStatusCode) const
{
    TRemoteDataReply reply;

    fetchReply(t_urlPath, reply);
    waitForReply(reply, t_requestTimeoutMsec);
    validateReply(reply);

    waitForFileDownload(reply);

    t_replyStatusCode = getReplyStatusCode(reply);

    return reply->readAll();
}

bool Downloader::doesStatusCodeIndicateSuccess(int t_statusCode)
{
    return t_statusCode >= 200 && t_statusCode < 300;
}

bool Downloader::checkInternetConnection()
{
    QProcess proc;
    QString exec = "ping";
    QStringList params = {Config::pingTarget, Config::pingCountArg, "1"};

    proc.start(exec, params);

    if (!proc.waitForFinished())
    {
        return false;
    }

    if (proc.exitCode() == 0)
    {
        return true;
    }
    else
    {
        logWarning("No internet connection.");
        QString p_stdout = proc.readAllStandardOutput();
        logInfo("Ping output was: %1", .arg(p_stdout));

        return false;
    }
}

void Downloader::abort()
{
    emit terminate();
}

void Downloader::onDownloadProgressChanged(const Downloader::TByteCount& t_bytesDownloaded, const Downloader::TByteCount& t_totalBytes)
{
    emit downloadProgressChanged(t_bytesDownloaded, t_totalBytes);
}

void Downloader::fetchReply(const QString& t_urlPath, TRemoteDataReply& t_reply) const
{
    QUrl url(t_urlPath);

    fetchReply(QNetworkRequest(url), t_reply);
}

void Downloader::fetchReply(const QNetworkRequest& t_urlRequest, TRemoteDataReply& t_reply) const
{
    logInfo("Fetching network reply - URL: %1.", .arg(t_urlRequest.url().toString()));

    if (!t_reply.isNull())
    {
        t_reply->deleteLater();
        t_reply.reset(nullptr);
    }

    if (!m_remoteDataSource)
    {
        throw std::runtime_error("No remote data source provided.");
    }

    QNetworkReply* reply = m_remoteDataSource->get(t_urlRequest);

    if (!reply)
    {
        throw std::runtime_error("Reply was null.");
    }

    t_reply = TRemoteDataReply(reply);
}

void Downloader::waitForReply(TRemoteDataReply& t_reply, int t_requestTimeoutMsec) const
{
    logInfo("Waiting for network reply to be ready.");

    if (t_reply->isFinished())
    {
        return;
    }

    if (!t_reply->waitForReadyRead(10))
    {
        QEventLoop readyReadLoop;

        QTimer timeoutTimer;

        connect(t_reply.data(), &QNetworkReply::readyRead, &readyReadLoop, &QEventLoop::quit);
        connect(&m_cancellationToken, &CancellationToken::cancelled, &readyReadLoop, &QEventLoop::quit);
        connect(&timeoutTimer, &QTimer::timeout, &readyReadLoop, &QEventLoop::quit);

        timeoutTimer.setInterval(t_requestTimeoutMsec);
        timeoutTimer.setSingleShot(true);
        timeoutTimer.start();

        readyReadLoop.exec();

        m_cancellationToken.throwIfCancelled();

        if (!timeoutTimer.isActive())
        {
            throw TimeoutException();
        }
    }
}

void Downloader::validateReply(TRemoteDataReply& t_reply) const
{
    logInfo("Validating network reply.");

    if (t_reply->error() != QNetworkReply::NoError)
    {
        throw std::runtime_error(t_reply->errorString().toStdString());
    }
}

int Downloader::getReplyStatusCode(TRemoteDataReply& t_reply) const
{
    QVariant statusCode = t_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (!statusCode.isValid())
    {
        throw std::runtime_error("Couldn't read HTTP status code from reply.");
    }

    int statusCodeValue = statusCode.toInt();

    logDebug("Reply status code - %1", .arg(statusCodeValue));

    return statusCodeValue;
}

void Downloader::waitForFileDownload(TRemoteDataReply& t_reply) const
{
    logInfo("Waiting for file download.");

    if (t_reply->isFinished())
    {
        return;
    }

    QEventLoop finishedLoop;

    connect(t_reply.data(), &QNetworkReply::finished, &finishedLoop, &QEventLoop::quit);
    connect(&m_cancellationToken, &CancellationToken::cancelled, &finishedLoop, &QEventLoop::quit);

    finishedLoop.exec();

    m_cancellationToken.throwIfCancelled();
}

void Downloader::restartDownload(TRemoteDataReply& t_reply, const QUrl& t_url) const
{
    QNetworkRequest request(t_url);
    restartDownload(t_reply, request);
}

void Downloader::restartDownload(TRemoteDataReply& t_reply, const QNetworkRequest& t_request) const
{
    disconnect(t_reply.data(), &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgressChanged);
    disconnect(this, &Downloader::terminate, t_reply.data(), &QNetworkReply::abort);

    // Fetch a new reply
    fetchReply(t_request, t_reply);

    connect(t_reply.data(), &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgressChanged);
    connect(this, &Downloader::terminate, t_reply.data(), &QNetworkReply::abort);
}

