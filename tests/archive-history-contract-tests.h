#ifndef MDPS_HISTORY_CONTRACT_TESTS_H
#define MDPS_HISTORY_CONTRACT_TESTS_H
// Executable regressions against the production history/transaction validators.
// Every filesystem mutation below is confined to a QTemporaryDir fixture.
#include "../src/archive/archivesafety.h"
#include <QTemporaryDir>
#include <stdexcept>
#include <functional>

namespace archive_history_tests {
inline void require(bool ok,const QString& why)
{
    if(!ok)throw std::runtime_error(why.toStdString());
}
inline QByteArray encode(const QJsonObject& object)
{
    return QJsonDocument(object).toJson(QJsonDocument::Compact)+"\n";
}
inline QJsonObject event(QString name="first_seen")
{
    return {{"schema_version",1},{"timestamp","2026-09-19T12:30:45.123+05:30"},
        {"event",name},{"item_key","youtube:abc123DEF45"},
        {"entry_key","youtube:abc123DEF45#1"},{"position",1},{"title","A title"}};
}
inline QByteArray read(const QString& path)
{
    QFile file(path);require(file.open(QIODevice::ReadOnly),"read fixture "+path);
    const auto bytes=file.readAll();require(file.error()==QFileDevice::NoError,"read fixture bytes");return bytes;
}
inline void write(const QString& path,const QByteArray& bytes)
{
    QFile file(path);require(file.open(QIODevice::WriteOnly),"write fixture "+path);
    require(file.write(bytes)==bytes.size()&&file.flush(),"write fixture bytes");
}
struct Fixture {
    QTemporaryDir temp;
    archive::Paths paths{temp.path()};
    archive::Store store{paths};
    archive::Source source;
    archive::Snapshot snapshot;
    Fixture()
    {
        QString error;require(temp.isValid()&&store.initialize(&error),"initialize fixture: "+error);
        source.key="PLHISTORY";source.url="https://www.youtube.com/playlist?list=PLHISTORY";
        archive::PlaylistItem item;item.providerId="abc123DEF45";item.itemKey="youtube:abc123DEF45";
        item.title="Fixture title";item.position=1;item.availability="public";
        snapshot.sourceKey=source.key;snapshot.complete=true;snapshot.items={item};
        require(store.reconcile(source,snapshot).committed,"reconcile history fixture");
    }
    QMap<QString,QByteArray> state() const
    {
        QMap<QString,QByteArray> result;
        for(const auto& path:QStringList{paths.itemsFile(),paths.sourcesFile(),
            paths.playlistItemsFile(source.key),paths.playlistHistoryFile(source.key)})result[path]=read(path);
        return result;
    }
};
inline bool run(QString* error)
{
    try {
        using archive::detail::historyValid;
        using archive::detail::transactionPayload;
        QString reason;
        QVector<QJsonObject> valid{event(),event("membership_reappeared")};
        auto removed=event("membership_removed");removed["last_position"]=-1;valid.append(removed);
        auto changed=event("observation_changed");
        changed["availability"]="public";changed["previous_availability"]="deleted";
        changed["previous_title"]="[Deleted video]";changed["previous_position"]=-1;valid.append(changed);
        auto promoted=event("identity_promoted");
        promoted["from_item_key"]="placeholder:PLHISTORY:1234567890abcdef:occurrence-1234abcd";
        promoted["to_item_key"]=promoted.value("item_key");valid.append(promoted);
        QJsonObject migrated{{"schema_version",1},{"timestamp","2026-09-19T12:30:45"},
            {"event","playlist_entry_key_migrated"},{"item_key",""},{"source_key","PLHISTORY"},
            {"rows",2},{"method","item_key_ordinal"}};valid.append(migrated);
        auto recovered=event("external_recovery_accepted");
        recovered["package_id"]="recovery-1";recovered["manifest_sha256"]=QString(64,'a');
        recovered["promoted_paths"]=QJsonArray{};valid.append(recovered); // Metadata-only import.
        recovered["promoted_paths"]=QJsonArray{"Video/item.mp4","Audio/item.m4a"};valid.append(recovered);
        QByteArray stream;
        for(const auto& record:valid){
            const auto bytes=encode(record);require(historyValid(bytes,&reason),"valid event rejected: "+reason);
            require(transactionPayload("Playlists/PLHISTORY/history.jsonl",bytes,&reason),"valid replay rejected: "+reason);
            stream+=bytes;
        }
        require(historyValid(stream,&reason),"valid mixed stream rejected: "+reason);
        require(historyValid("\n\r\n"+stream,&reason),"historical blank lines rejected");
        auto crlf=stream;crlf.replace("\n","\r\n");require(historyValid(crlf,&reason),"CRLF history rejected");
        require(historyValid({},&reason),"empty history rejected");
        for(const auto& timestamp:QStringList{"2026-09-19T12:30:45Z","2026-09-19T12:30:45.123",
                "2026-09-19T12:30:45-04:00"}){
            auto record=event();record["timestamp"]=timestamp;
            require(historyValid(encode(record),&reason),"valid timestamp rejected: "+timestamp);
        }
        QVector<QPair<QString,QJsonObject>> invalid;
        for(const auto& field:QStringList{"schema_version","timestamp","event","item_key","entry_key","position","title"}){
            auto record=event();record.remove(field);invalid.append({"missing "+field,record});
        }
        const auto change=[&](const QString& label,const QJsonObject& original,const QString& field,const QJsonValue& value){
            auto record=original;record[field]=value;invalid.append({label,record});
        };
        for(const auto& value:QJsonArray{true,"1",2,1.5,QJsonValue::Null})change("schema type/version",event(),"schema_version",value);
        for(const auto& value:QJsonArray{7,"","not-a-date","2026-02-30T12:30:45Z","2026-09-19","2026-09-19T99:30:45Z"})
            change("timestamp contract",event(),"timestamp",value);
        for(const auto& field:QStringList{"event","item_key","entry_key"})
            for(const auto& value:QJsonArray{"","   ",5,true,QJsonValue::Null})change(field+" contract",event(),field,value);
        change("unknown event",event(),"event","future_event");
        for(const auto& value:QJsonArray{"1",true,1.25,-2,2147483648.0})change("position contract",event(),"position",value);
        change("title type",event(),"title",7);
        change("previous position type",changed,"previous_position","1");
        change("availability type",changed,"availability",7);
        change("previous title type",changed,"previous_title",false);
        change("removed position type",removed,"last_position",false);
        change("promotion target mismatch",promoted,"to_item_key","youtube:xyz987QWE65");
        change("promotion source empty",promoted,"from_item_key","");
        change("migration rows fractional",migrated,"rows",1.5);
        change("migration rows missing",migrated,"rows",QJsonValue::Undefined);
        change("migration source invalid",migrated,"source_key","../bad");
        change("migration method unknown",migrated,"method","guess");
        change("migration aggregate identity",migrated,"item_key","youtube:abc123DEF45");
        change("package identity invalid",recovered,"package_id","../bad");
        change("manifest digest malformed",recovered,"manifest_sha256","not-a-digest");
        change("promoted paths type",recovered,"promoted_paths","Video/item.mp4");
        change("promoted path traversal",recovered,"promoted_paths",QJsonArray{"../item.mp4"});
        change("promoted path type",recovered,"promoted_paths",QJsonArray{42});
        invalid.append({"empty object",QJsonObject{}});
        for(const auto& pair:invalid){
            const auto bytes=encode(pair.second);reason.clear();
            require(!historyValid(bytes,&reason),"semantically corrupt event accepted: "+pair.first);
            require(!reason.isEmpty(),"rejection lacked a diagnostic: "+pair.first);
            require(!transactionPayload("Playlists/PLHISTORY/history.jsonl",bytes,&reason),"corrupt replay accepted: "+pair.first);
            require(!historyValid(stream+bytes,&reason),"corrupt tail accepted: "+pair.first);
        }
        require(!historyValid(stream.left(stream.size()-1),&reason),"truncated tail accepted");
        require(!historyValid("not-json\n",&reason),"invalid JSON accepted");
        require(!historyValid("[]\n",&reason),"non-object history accepted");
        // Exercise the public append path, not only the pure validator.
        {
            Fixture fixture;const auto before=fixture.state();
            require(!fixture.store.appendHistory(fixture.source.key,{},&reason),"append accepted invalid event");
            require(fixture.state()==before,"rejected append modified canonical state");
            require(fixture.store.appendHistory(fixture.source.key,event(),&reason),"valid append failed: "+reason);
            require(historyValid(read(fixture.paths.playlistHistoryFile(fixture.source.key)),&reason),"append produced invalid stream");
        }
        // Existing semantic corruption must not be silently extended or rewritten.
        for(const auto& corruption:QVector<QByteArray>{"{}\n",encode(invalid.first().second),encode(invalid.last().second)}){
            Fixture fixture;const auto history=fixture.paths.playlistHistoryFile(fixture.source.key);
            write(history,corruption);const auto before=fixture.state();
            require(!fixture.store.appendHistory(fixture.source.key,event(),&reason),"append extended corrupt history");
            require(fixture.state()==before,"append changed corrupt history or registry");
            require(!fixture.store.reconcile(fixture.source,fixture.snapshot).committed,"scan accepted corrupt history");
            require(fixture.state()==before,"scan changed corrupt history or registry");
            require(!fixture.store.initialize(&reason),"restart accepted corrupt history");
            require(fixture.state()==before,"restart changed corrupt history or registry");
            require(!QFileInfo::exists(archive::detail::journalPath(fixture.paths.root())),"rejection left a publication journal");
        }
        // All transaction payloads are checked before any roll-forward write.
        {
            Fixture fixture;const auto before=fixture.state();
            const auto rel="Playlists/"+fixture.source.key+"/history.jsonl";
            const auto after=QByteArray("{}\n");
            const QJsonObject operation{{"path",rel},{"before_exists",true},
                {"before_sha256",archive::detail::digest(read(fixture.paths.playlistHistoryFile(fixture.source.key)))},
                {"after_base64",QString::fromLatin1(after.toBase64())},{"after_sha256",archive::detail::digest(after)}};
            const auto journal=archive::detail::journalPath(fixture.paths.root());
            const auto bytes=encode(QJsonObject{{"schema_version",1},{"operations",QJsonArray{operation}}});
            write(journal,bytes);
            require(!archive::detail::recoverTransaction(fixture.paths.root(),&reason),"replay published semantic corruption");
            require(reason.contains("history",Qt::CaseInsensitive),"replay failed for an unrelated reason: "+reason);
            require(fixture.state()==before&&read(journal)==bytes,"replay changed state or destroyed repair evidence");
        }
        QTextStream(stdout)<<"history-contract: "<<valid.size()<<" valid event forms, "<<invalid.size()
            <<" semantic rejection cases, append/scan/restart/replay preservation passed\n";
        return true;
    }catch(const std::exception& ex){if(error)*error=QString::fromUtf8(ex.what());return false;}
}
}
#endif
