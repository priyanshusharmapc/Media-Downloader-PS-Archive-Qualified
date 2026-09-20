#pragma once

#include <QJsonValue>
#include <QString>
#include <QtGlobal>

#include <cmath>

namespace engineJson
{
inline bool nonNegativeByteCount(const QJsonValue& value,qint64* out)
{
    if(out==nullptr)return false;

    if(value.isString()){
        bool ok=false;
        const auto parsed=value.toString().toLongLong(&ok);
        if(!ok||parsed<0)return false;
        *out=parsed;return true;
    }

    if(!value.isDouble())return false;

    const auto number=value.toDouble();
    // QJson stores numbers as double. Refuse integers above 2^53-1 rather
    // than silently rounding a backend byte count before it reaches qint64.
    constexpr double largestExactJsonInteger=9007199254740991.0;
    if(!std::isfinite(number)||number<0||std::floor(number)!=number||
       number>largestExactJsonInteger)return false;

    *out=static_cast<qint64>(number);
    return true;
}

inline qint64 nonNegativeByteCount(const QJsonValue& value)
{
    qint64 result=0;
    return nonNegativeByteCount(value,&result)?result:0;
}
}
