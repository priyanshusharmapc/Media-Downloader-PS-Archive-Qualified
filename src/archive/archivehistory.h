#ifndef MDPS_ARCHIVEHISTORY_H
#define MDPS_ARCHIVEHISTORY_H
// Versioned contract for canonical playlist history, independent of activity logs.
// Validation never edits the supplied bytes. Unknown versions/events must stop a
// writer so historical evidence is preserved for an explicit repair or upgrade.
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringList>
#include <cmath>
#include <limits>

namespace archive { namespace detail {
inline bool historyEventValid(const QJsonObject& object,QString* error)
{
    const auto rejectEvent=[&](const QString& why){if(error)*error=why;return false;};
    const auto text=[&](const QString& key,bool nonempty=false){
        const auto value=object.value(key);
        return value.isString()&&(!nonempty||!value.toString().trimmed().isEmpty());
    };
    const auto integer=[&](const QString& key,int minimum){
        const auto value=object.value(key);const auto number=value.toDouble();
        return value.isDouble()&&std::isfinite(number)&&number>=minimum&&
            number<=std::numeric_limits<int>::max()&&std::floor(number)==number;
    };
    // QJsonValue equality avoids permissive string/bool/integer coercions.
    if(object.value("schema_version")!=QJsonValue(1))
        return rejectEvent("Unsupported or missing history schema_version; expected number 1");
    const auto timestamp=object.value("timestamp").toString();
    static const QRegularExpression timestampShape(
        "^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}(?:\\.[0-9]+)?(?:Z|[+-][0-9]{2}:[0-9]{2})?$");
    if(!text("timestamp",true)||!timestampShape.match(timestamp).hasMatch()||
       !QDateTime::fromString(timestamp,Qt::ISODateWithMs).isValid())
        return rejectEvent("Invalid history timestamp; expected an ISO date and time");
    if(!text("event",true)||!text("item_key"))
        return rejectEvent("History event and item_key must be strings");
    const auto event=object.value("event").toString();
    const auto itemKey=object.value("item_key").toString();
    static const QRegularExpression sourceKey("^[A-Za-z0-9_-]{1,160}$");
    if(event=="playlist_entry_key_migrated"){
        // This is a playlist-wide event, not a canonical item event. The current
        // migration deliberately writes an empty item_key and must remain valid.
        if(!itemKey.isEmpty()||!text("source_key",true)||
           !sourceKey.match(object.value("source_key").toString()).hasMatch()||
           !integer("rows",1)||object.value("method")!="item_key_ordinal")
            return rejectEvent("Invalid playlist occurrence-identity migration event");
        return true;
    }
    if(itemKey.trimmed().isEmpty())return rejectEvent("Item history event has an empty item_key");
    if(event=="external_recovery_accepted"){
        static const QRegularExpression digest("^[0-9a-f]{64}$");
        if(!text("package_id",true)||!sourceKey.match(object.value("package_id").toString()).hasMatch()||
           !text("manifest_sha256",true)||!digest.match(object.value("manifest_sha256").toString()).hasMatch()||
           !object.value("promoted_paths").isArray())
            return rejectEvent("Invalid recovery history package, digest or promoted_paths");
        // Empty promoted_paths is intentional for a metadata-only recovery.
        // These are historical references, so do not test current file existence.
        for(const auto& value:object.value("promoted_paths").toArray()){
            if(!value.isString())return rejectEvent("History promoted path must be a string");
            const auto path=value.toString();const auto parts=path.split('/');
            if((!path.startsWith("Video/")&&!path.startsWith("Audio/"))||path.contains('\\')||
               parts.contains("")||parts.contains(".")||parts.contains(".."))
                return rejectEvent("History promoted path is not a canonical media reference");
        }
        return true;
    }
    if(!text("entry_key",true))return rejectEvent("Occurrence history event requires entry_key");
    if(event=="first_seen"){
        if(!integer("position",-1)||!text("title"))return rejectEvent("Invalid first_seen event");
    }else if(event=="membership_reappeared"){
        if(!integer("position",-1))return rejectEvent("Invalid membership_reappeared position");
    }else if(event=="membership_removed"){
        if(!integer("last_position",-1)||!text("title"))return rejectEvent("Invalid membership_removed event");
    }else if(event=="observation_changed"){
        // -1 is the established unknown-position sentinel. Availability remains
        // an open string vocabulary because provider states may be unfamiliar.
        if(!integer("position",-1)||!integer("previous_position",-1)||!text("title")||
           !text("previous_title")||!text("availability")||!text("previous_availability"))
            return rejectEvent("Invalid observation_changed field types");
    }else if(event=="identity_promoted"){
        if(!integer("position",-1)||!text("from_item_key",true)||!text("to_item_key",true)||
           object.value("to_item_key").toString()!=itemKey||object.value("from_item_key").toString()==itemKey)
            return rejectEvent("Invalid canonical identity promotion event");
    }else return rejectEvent("Unsupported history event: "+event);
    return true;
}

inline bool historyValid(const QByteArray& bytes,QString* error)
{
    const auto rejectHistory=[&](const QString& why){if(error)*error=why;return false;};
    if(!bytes.isEmpty()&&!bytes.endsWith('\n'))return rejectHistory("Incomplete history record; original history was preserved");
    int lineNumber=0;
    for(const auto& line:bytes.split('\n')){
        ++lineNumber;
        // Earlier writers allowed blank lines and Windows CRLF. Keep them
        // readable without reserializing or removing any existing evidence.
        if(line.trimmed().isEmpty())continue;
        QJsonParseError parseError;const auto document=QJsonDocument::fromJson(line,&parseError);
        QString reason;
        if(parseError.error!=QJsonParseError::NoError||!document.isObject())reason="expected a JSON object";
        else if(historyEventValid(document.object(),&reason))continue;
        return rejectHistory(QString("Invalid history record at line %1: %2; original history was preserved").arg(lineNumber).arg(reason));
    }
    return true;
}
}}
#endif
