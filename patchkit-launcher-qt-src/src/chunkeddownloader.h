/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#ifndef CHUNKEDDOWNLOADER_H
#define CHUNKEDDOWNLOADER_H

#include <QObject>
#include <QVector>
#include <QStack>

#include "downloader.h"

#include "hashingstrategy.h"

class ContentSummary;

class QNetworkReply;
class QNetworkRequest;
class QNetworkAccessManager;

/**
 * @brief
 * Downloads files in chunks which are specified by the Content Summary.
 *
 * @details
 * As it is implemented now, the Chunked Downloader downloads the data through the QNetworkReply.
 * When the connection is broken or for any other reason the download has stopped, it validates all the downloaded data.
 * Once the data is validated the ChunkedDownloader proceeds to restart the download from the first invalid chunk (or from the start if no chunks were received)
 *
 * The t_staleDownloadTimeoutMsec parameter (passed in constructor) controls the time needed to cause
 * the stale download exception - if no good chunks have been downloaded in this time, the download will terminate
 * and an exception will be thrown.
 *
 * @note
 * This means that if in a set of 20 chunks the chunk #5 happened to be invalid but all the other ones were valid
 * the download will resume from the 5th chunk.
 */
class ChunkedDownloader : public Downloader
{
    Q_OBJECT

public:
    ChunkedDownloader(
            QNetworkAccessManager* t_dataSource,
            const ContentSummary& t_contentSummary,
            HashFunc t_hashingStrategy,
            CancellationToken t_cancellationToken
            );

    QByteArray downloadFile(const QString& t_urlPath, int t_requestTimeoutMsec, int* t_replyStatusCode = nullptr) override;

public slots:
    virtual void abort() override;

protected slots:
    virtual void onDownloadProgressChanged(const TByteCount& t_bytesDownloaded, const TByteCount& t_totalBytes) override;

private:
    bool                    m_running;

    QVector<QByteArray>     m_chunks;
    int                     m_lastValidChunkIndex;
    HashFunc                m_hashingStrategy;
    const ContentSummary&   m_contentSummary;

    QVector<QByteArray> processChunks(QByteArray& t_data) const;

    bool        validateReceivedData(QByteArray& t_data);
    const int   getChunkSize() const;

    bool        shouldStop() const;
};

#endif // CHUNKEDDOWNLOADER_H
