/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#ifndef MOCKEDREPLY_H
#define MOCKEDREPLY_H

#include <QNetworkReply>

class MockedNetworkReply : public QNetworkReply
{
    Q_OBJECT

public:
    MockedNetworkReply(int t_delayMsec, QByteArray t_data, int t_statusCode, QObject* parent = nullptr);

    void setContent(const QString& m_content);
    void setContent(const QByteArray& m_content);

    void setOffset(qint64 t_offset);

    void launch();
    void corrupt();

    void abort();
    qint64 bytesAvailable() const;
    bool isSequential() const;

protected:
    qint64 readData(char* data, qint64 maxSize);

private:
    QByteArray m_content;
    qint64 m_contentOffset;

    int m_statusCode;
    int m_replyDelayMsec;
};

#endif // MOCKEDREPLY_H
