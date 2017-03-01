#ifndef IAPI_H
#define IAPI_H

class IApi
{
public:
    virtual QString downloadString(const QString& t_apiUrl, const QString& t_resourceUrl, CancellationToken t_cancellationToken) const = 0;

    // Returns all the api urls (the main and cache), main url should always be at the 0 th index.
    virtual const QStringList& getApiUrls() const = 0;
};

#endif // IAPI_H
