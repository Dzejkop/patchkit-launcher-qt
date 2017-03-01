/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#ifndef MOCKEDNAM_H
#define MOCKEDNAM_H

#include "mockedreply.h"

#include <QNetworkAccessManager>
#include <QtNetwork>
#include <QMap>

class MockedNAM : public QNetworkAccessManager
{
public:
    MockedNAM();

    void push(QString t_url, QByteArray t_data, int t_replyDelay, int t_statusCode = 200);
    void purge();
    int  timesUrlAccessed(QString url) const;
    void corrupt();
    void corrupt(int t_amount);

protected:
    virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest& request, QIODevice* outgoingData) override;

private:
    struct ReplyDefinition
    {
        ReplyDefinition(QByteArray t_data, int t_delay, int t_statusCode = 200)
            : data(t_data)
            , delay(t_delay)
            , timesAccesed(0)
            , statusCode(t_statusCode)
        {
        }

        QByteArray data;
        int delay;

        int statusCode;
        int timesAccesed;
    };

    QByteArray m_data;
    int m_replyDelay;

    int m_repliesToCorrupt;

    qint64 parseRangeHeader(const QByteArray& t_rangeHeader);

    QMap<QString, ReplyDefinition> m_replyDefinitions;
};

#endif // MOCKEDNAM_H
