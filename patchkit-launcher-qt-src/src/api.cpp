/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#include "api.h"

#include "downloader.h"
#include "timeoutexception.h"
#include "config.h"

#include "contentsummary.h"

Api::Api(QObject* parent) : QObject(parent)
{
}

QString Api::downloadString(const QString& t_resourceUrl, CancellationToken t_cancellationToken) const
{
    QStringList cacheApiUrls = Config::cacheApiUrls;
    return downloadString(t_resourceUrl, cacheApiUrls, false, t_cancellationToken);
}

QJsonDocument Api::downloadContentSummary(const QString& t_resourceUrl, CancellationToken t_cancellationToken) const
{
    const auto validator = [](const QString& data) -> bool
    {
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
        return !doc.isNull() && !doc.isEmpty();
    };

    QStringList cacheApiUrls = Config::cacheApiUrls;

    QString raw = downloadString(t_resourceUrl, cacheApiUrls, validator, false, t_cancellationToken);
    return QJsonDocument::fromJson(raw.toUtf8());
}

QString Api::downloadString(const QString& t_resourceUrl, QStringList& t_cacheApiUrls, bool t_extendedTimeout, CancellationToken t_cancellationToken) const
{
    return downloadString(t_resourceUrl, t_cacheApiUrls, nullptr, t_extendedTimeout, t_cancellationToken);
}

QString Api::downloadString(const QString& t_resourceUrl, QStringList& t_cacheApiUrls, TValidator t_validator, bool t_extendedTimeout, CancellationToken t_cancellationToken) const
{
    QString result;
    int statusCode;

    int timeout = t_extendedTimeout ? Config::maxConnectionTimeoutMsec : Config::minConnectionTimeoutMsec;

    if (downloadStringFromServer(Config::mainApiUrl + "/" + t_resourceUrl, timeout, result, statusCode, t_cancellationToken))
    {
        if (!isVaild(statusCode))
        {
            throw std::runtime_error("API response error. Status code - " + std::to_string(statusCode));
        }

        if (!t_validator || (t_validator && t_validator(result)))
            return result;
    }
    else
    {
        emit connectionIssue(t_extendedTimeout);
    }


    for (int i = 0; i < t_cacheApiUrls.length(); i++)
    {
        if (downloadStringFromServer(t_cacheApiUrls[i] + "/" + t_resourceUrl, timeout, result, statusCode, t_cancellationToken))
        {
            if (isVaild(statusCode))
            {
                if (!t_validator || (t_validator && t_validator(result)))
                    return result;
            }

            t_cacheApiUrls.removeAt(i);
            i--;
        }
        else
        {
            emit connectionIssue(i >= 4 || t_extendedTimeout);
        }
    }

    if (t_extendedTimeout)
    {
        throw std::runtime_error("API connection error.");
    }

    return downloadString(t_resourceUrl, t_cacheApiUrls, t_validator, true, t_cancellationToken);
}

bool Api::isVaild(int t_statusCode) const
{
    return t_statusCode == 200;
}

bool Api::downloadStringFromServer(const QString& t_url, int t_timeout, QString& t_result, int& t_statusCode, CancellationToken t_cancellationToken) const
{
    QNetworkAccessManager remoteDataSource;

    try
    {
        Downloader downloader(&remoteDataSource, t_cancellationToken);
        t_result = downloader.downloadString(t_url, t_timeout, t_statusCode);

        if (t_statusCode == 500)
        {
            return false;
        }

        return true;
    }
    catch (TimeoutException&)
    {
        return false;
    }
}
