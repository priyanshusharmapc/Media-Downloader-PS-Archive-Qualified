#pragma once

#include <QJsonValue>
#include <QString>
#include <QtGlobal>

#include <cmath>

namespace engineJson
{
inline qint64 nonNegativeByteCount(const QJsonValue& value)
{
    if(value.isString()){
        bool ok=false;
        const auto parsed=value.toString().toLongLong(&ok);
        return ok&&parsed>=0?parsed:0;
    }

    if(!value.isDouble())return 0;

    const auto number=value.toDouble();
    // QJson stores numbers as double. Refuse integers above 2^53-1 rather
    // than silently rounding a backend byte count before it reaches qint64.
    constexpr double largestExactJsonInteger=9007199254740991.0;
    if(!std::isfinite(number)||number<0||std::floor(number)!=number||
       number>largestExactJsonInteger)return 0;

    return static_cast<qint64>(number);
}
}
