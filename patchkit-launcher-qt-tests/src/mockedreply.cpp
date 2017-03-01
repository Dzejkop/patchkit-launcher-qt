/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#include "mockedreply.h"

#include <QTimer>
#include <QtGlobal>

MockedNetworkReply::MockedNetworkReply(int t_delayMsec, QByteArray t_data, int t_statusCode, QObject* t_parent)
    : QNetworkReply(t_parent)
    , m_replyDelayMsec(t_delayMsec)
    , m_statusCode(t_statusCode)
{
    setContent(t_data);
}

void MockedNetworkReply::setContent(const QString& t_conent)
{
    setContent(t_conent.toUtf8());
}

void MockedNetworkReply::setContent(const QByteArray& t_conent)
{
    m_content = t_conent;
    m_contentOffset = 0;

    setHeader(QNetworkRequest::ContentLengthHeader, QVariant(t_conent.size()));
}

void MockedNetworkReply::setOffset(qint64 t_offset)
{
    m_contentOffset = t_offset;
}

void MockedNetworkReply::launch()
{
    open(ReadOnly | Unbuffered);
    QTimer::singleShot( m_replyDelayMsec, this, [&]()
    {
        this->setFinished(true);
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, QVariant::fromValue(m_statusCode));
        emit readyRead();
        emit finished();
    });
}

void MockedNetworkReply::corrupt()
{
    m_content[rand() % m_content.size()] = rand();
}

void MockedNetworkReply::abort()
{
}

qint64 MockedNetworkReply::bytesAvailable() const
{
    return m_content.size() - m_contentOffset;
}

bool MockedNetworkReply::isSequential() const
{
    return true;
}

qint64 MockedNetworkReply::readData(char* t_data, qint64 t_maxSize)
{
    if (m_contentOffset >= m_content.size())
    {
        return -1;
    }

    qint64 readSize = qMin(t_maxSize, m_content.size() - m_contentOffset);
    memcpy(t_data, m_content.constData() + m_contentOffset, readSize);
    m_contentOffset += readSize;

    return readSize;
}
