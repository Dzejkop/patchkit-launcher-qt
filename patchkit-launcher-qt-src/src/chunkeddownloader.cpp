/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#include <QtNetwork>

#include "chunkeddownloader.h"

#include "timeoutexception.h"
#include "contentsummary.h"
#include "downloader.h"
#include "logger.h"
#include "staledownloadexception.h"

ChunkedDownloader::ChunkedDownloader(
        QNetworkAccessManager* t_dataSource,
        const ContentSummary& t_contentSummary,
        HashFunc t_hashingStrategy,
        CancellationToken t_cancellationToken
        )
    : Downloader(t_dataSource, t_cancellationToken)
    , m_contentSummary(t_contentSummary)
    , m_lastValidChunkIndex(0)
    , m_hashingStrategy(t_hashingStrategy)
    , m_running(true)
{
}

QByteArray ChunkedDownloader::downloadFile(const QString& t_urlPath, int t_requestTimeoutMsec, int* t_replyStatusCode)
{
    if (!m_hashingStrategy)
    {
        throw std::runtime_error("No hashing strategy specified.");
    }

    QByteArray data;
    QNetworkRequest request;

    QUrl url(t_urlPath);

    request = QNetworkRequest(url);

    m_running = true;

    int replyStatusCode = -1;

    while (!shouldStop())
    {
        try
        {
            data = Downloader::downloadFile(request, t_requestTimeoutMsec, &replyStatusCode);

            if (t_replyStatusCode != nullptr)
            {
                *t_replyStatusCode = replyStatusCode;
            }

            if (!doesStatusCodeIndicateSuccess(replyStatusCode))
            {
                throw std::runtime_error(QString("Chunked download failed, status code was %1.").arg(replyStatusCode).toStdString());
            }

            if (validateReceivedData(data))
            {
                break;
            }
            else
            {
                int startIndex = m_lastValidChunkIndex + 1;
                if (m_lastValidChunkIndex == 0)
                {
                    startIndex = 0;
                }

                QByteArray header = "bytes=" + QByteArray::number(startIndex * getChunkSize()) + "-";

                logInfo("Reformulating request URL: %1, Range header: %2", .arg(url.toString(), (QString)header));

                request = QNetworkRequest(url);
                request.setRawHeader("Range", header);
            }
        }
        catch (CancelledException&)
        {
            throw;
        }
        catch (TimeoutException&)
        {
            throw;
        }
    }

    QByteArray reassembledData;
    for (QByteArray chunk : m_chunks)
    {
        reassembledData += chunk;
    }

    return reassembledData;
}

void ChunkedDownloader::onDownloadProgressChanged(const TByteCount &t_bytesDownloaded, const TByteCount &t_totalBytes)
{
    TByteCount bytesInValidChunksDownloadedSoFar = m_lastValidChunkIndex * getChunkSize();
    emit Downloader::downloadProgressChanged(bytesInValidChunksDownloadedSoFar + t_bytesDownloaded, bytesInValidChunksDownloadedSoFar+ t_totalBytes);
}

bool ChunkedDownloader::shouldStop() const
{
    return !m_running;
}

void ChunkedDownloader::abort()
{
    Downloader::abort();

    m_running = false;
}

QVector<QByteArray> ChunkedDownloader::processChunks(QByteArray& t_data) const
{
    QVector<QByteArray> chunks;

    const int chunkSize = getChunkSize();
    for (int i = 0; i < t_data.size(); i += chunkSize)
    {
        // Get the chunk
        QByteArray chunk = t_data.mid(i, chunkSize);

        chunks.push_back(chunk);
    }

    return chunks;
}

bool ChunkedDownloader::validateReceivedData(QByteArray& t_data)
{
    QVector<QByteArray> chunks = processChunks(t_data);

    m_chunks += chunks;

    for (int i = m_lastValidChunkIndex; i < m_chunks.size(); i++)
    {
        // Get the content summary hash
        THash valid_hash = m_contentSummary.getChunkHash(i);

        // At this point the m_hashingStrategy is guaranteed to be valid, no need to check if it's null.
        THash hash = m_hashingStrategy(m_chunks.at(i));

        if (valid_hash != hash)
        {
            // Mark the previous index as the last valid
            m_lastValidChunkIndex = i - 1;

            if (m_lastValidChunkIndex < 0)
                m_lastValidChunkIndex = 0;

            // Rend the remaining invalid chunks
            m_chunks.remove(i, m_chunks.size() - i);

            // Signal to restart download
            return false;
        }
        else
        {
            m_lastValidChunkIndex = i;
        }
    }

    return m_chunks.size() == m_contentSummary.getChunksCount();
}

const int ChunkedDownloader::getChunkSize() const
{
    return m_contentSummary.getChunkSize();
}
