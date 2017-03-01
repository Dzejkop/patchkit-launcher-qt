#ifndef APIOPERATOR_H
#define APIOPERATOR_H

#include <QJsonDocument>

#include "iapi.h"

class ApiOperator
{
public:
    ApiOperator(const IApi& t_api);

    // Configuration options
    void setMinTimeout(int t_timeout);
    void setMaxTimeout(int t_timeout);
    void setTimeoutCountTolerance(int t_tolerance);

    // Operations
    QJsonDocument downloadContentSummary();

private:

    const IApi& m_api;
};

#endif // APIOPERATOR_H
