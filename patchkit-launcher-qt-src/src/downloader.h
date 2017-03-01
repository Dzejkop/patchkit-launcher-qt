/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#pragma once

#include <QtNetwork>
#include <memory>

#include "cancellationtoken.h"

class Downloader : public QObject
{
    Q_OBJECT

public:
    Downloader(QNetworkAccessManager* t_dataSource, CancellationToken& t_cancellationToken);

    typedef QSharedPointer<QNetworkReply>          TRemoteDataReply;

    typedef long long TByteCount;

    virtual QByteArray  downloadFile(const QString& t_urlPath, int t_requestTimeoutMsec, int* t_replyStatusCode = nullptr);
    QString downloadString(const QString& t_urlPath, int t_requestTimeoutMsec, int& t_replyStatusCode) const;

    static bool doesStatusCodeIndicateSuccess(int t_statusCode);
    static bool checkInternetConnection();

signals:
    void downloadProgressChanged(const TByteCount& t_bytesDownloaded, const TByteCount& t_totalBytes);
    void terminate();

public slots:
    virtual void abort();

protected slots:
    virtual void onDownloadProgressChanged(const TByteCount& t_bytesDownloaded, const TByteCount& t_totalBytes);

protected:
    QByteArray downloadFile(const QNetworkRequest& t_request, int t_requestTimeoutMsec, int* t_replyStatusCode = nullptr);

    void fetchReply(const QString& t_urlPath, TRemoteDataReply& t_reply) const;
    void fetchReply(const QNetworkRequest& t_urlRequest, TRemoteDataReply& t_reply) const;

    void waitForReply(TRemoteDataReply& t_reply, int t_requestTimeoutMsec) const;

    void validateReply(TRemoteDataReply& t_reply) const;

    int  getReplyStatusCode(TRemoteDataReply& t_reply) const;

    void waitForFileDownload(TRemoteDataReply& t_reply) const;

    void restartDownload(TRemoteDataReply& t_reply, const QUrl& t_url) const;
    void restartDownload(TRemoteDataReply& t_reply, const QNetworkRequest& t_request) const;

    CancellationToken m_cancellationToken;

private:
    QNetworkAccessManager* m_remoteDataSource;
};
