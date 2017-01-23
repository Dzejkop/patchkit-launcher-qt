/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#include "downloader.h"

#include "logger.h"
#include "timeoutexception.h"

Downloader::Downloader(QNetworkAccessManager* t_dataSource)
    : m_remoteDataSource(t_dataSource)
{
}

QByteArray Downloader::downloadFile(const QString& t_urlPath, int t_requestTimeoutMsec, CancellationToken t_cancellationToken)
{
    QNetworkRequest request(t_urlPath);

    return downloadFile(request, t_requestTimeoutMsec, t_cancellationToken);
}

QByteArray Downloader::downloadFile(const QNetworkRequest& t_request, int t_requestTimeoutMsec, CancellationToken t_cancellationToken)
{
    TRemoteDataReply reply;

    fetchReply(t_request, reply);

    connect(reply.data(), &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgressChanged);

    waitForReply(reply, t_requestTimeoutMsec, t_cancellationToken);
    validateReply(reply);

    waitForFileDownload(reply, t_cancellationToken);

    disconnect(reply.data(), &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgressChanged);

    return reply->readAll();
}

QString Downloader::downloadString(const QString& t_urlPath, int t_requestTimeoutMsec, int& t_replyStatusCode, CancellationToken t_cancellationToken) const
{
    TRemoteDataReply reply;

    fetchReply(t_urlPath, reply);
    waitForReply(reply, t_requestTimeoutMsec, t_cancellationToken);
    validateReply(reply);

    waitForFileDownload(reply, t_cancellationToken);

    t_replyStatusCode = getReplyStatusCode(reply);

    return reply->readAll();
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
    logInfo("Fetching network reply.");

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

void Downloader::waitForReply(TRemoteDataReply& t_reply, int t_requestTimeoutMsec, CancellationToken t_cancellationToken) const
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
        connect(&t_cancellationToken, &CancellationToken::cancelled, &readyReadLoop, &QEventLoop::quit);
        connect(&timeoutTimer, &QTimer::timeout, &readyReadLoop, &QEventLoop::quit);

        timeoutTimer.setInterval(t_requestTimeoutMsec);
        timeoutTimer.setSingleShot(true);
        timeoutTimer.start();

        readyReadLoop.exec();

        t_cancellationToken.throwIfCancelled();

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

void Downloader::waitForFileDownload(TRemoteDataReply& t_reply, CancellationToken t_cancellationToken) const
{
    logInfo("Waiting for file download.");

    if (t_reply->isFinished())
    {
        return;
    }

    QEventLoop finishedLoop;

    connect(t_reply.data(), &QNetworkReply::finished, &finishedLoop, &QEventLoop::quit);
    connect(&t_cancellationToken, &CancellationToken::cancelled, &finishedLoop, &QEventLoop::quit);

    finishedLoop.exec();

    t_cancellationToken.throwIfCancelled();
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

