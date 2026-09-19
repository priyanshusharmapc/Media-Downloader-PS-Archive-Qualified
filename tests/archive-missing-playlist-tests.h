#ifndef MDPS_MISSING_PLAYLIST_TESTS_H
#define MDPS_MISSING_PLAYLIST_TESTS_H
#include "archive-history-contract-tests.h"
#include <QDirIterator>

namespace archive_missing_playlist_tests {
using archive_history_tests::require;
using archive_history_tests::read;
using archive_history_tests::write;
// Snapshot every surviving managed byte. Rejected repair must not normalize
// damaged state, drop old membership, or manufacture a replacement items file.
inline QMap<QString,QByteArray> snapshot(const QString& root)
{
    QMap<QString,QByteArray> result;
    QDirIterator it(root,QDir::Files|QDir::Hidden|QDir::System,QDirIterator::Subdirectories);
    while(it.hasNext()){
        const auto path=it.next();if(path.endsWith("/sync.lock"))continue;
        result[QDir(root).relativeFilePath(path)]=read(path);
    }
    return result;
}
inline bool run(QString* error)
{
    try {
        QString reason;
        // Registry admission, an optional fresh metadata record, and an
        // unregistered in-memory source must all retain their first-scan path.
        for(int admission=0;admission<4;++admission){
            QTemporaryDir temp;archive::Paths paths(temp.path());archive::Store store(paths);
            require(temp.isValid()&&store.initialize(&reason),"fresh fixture: "+reason);
            archive::Source source;source.key="PLFRESH";source.url="https://www.youtube.com/playlist?list=PLFRESH";
            if(admission>0)require(store.saveSources({source},&reason),"fresh source admission");
            if(admission==2)require(store.savePlaylistMeta(source,&reason),"fresh metadata");
            if(admission==3){
                QJsonObject legacy{{"key",source.key},{"url",source.url},{"title","Legacy first admission"}};
                write(paths.sourcesFile(),QJsonDocument(QJsonArray{legacy}).toJson());
            }
            reason.clear();require(store.loadPlaylistItems(source.key,&reason).isEmpty()&&reason.isEmpty(),"fresh absence rejected: "+reason);
            require(store.initialize(&reason),"fresh source restart: "+reason);
            archive::Snapshot observation;observation.sourceKey=source.key;observation.complete=true;
            require(store.reconcile(source,observation).committed,"empty first scan rejected");
            require(QFileInfo::exists(paths.playlistItemsFile(source.key)),"empty first scan did not establish state");
        }
        // Exercise independent lifecycle evidence, not only the happy-path
        // last_scan_status. A stale registry must not override retained history.
        for(int loss=0;loss<7;++loss){
            archive_history_tests::Fixture fixture;
            auto second=fixture.snapshot.items.first();second.providerId="xyz987QWE65";
            second.itemKey="youtube:xyz987QWE65";second.title="Historical B";second.position=2;
            fixture.snapshot.items.append(second);
            require(fixture.store.reconcile(fixture.source,fixture.snapshot).committed,"A+B setup");
            const auto originalItems=read(fixture.paths.playlistItemsFile(fixture.source.key));
            auto sources=QJsonDocument::fromJson(read(fixture.paths.sourcesFile())).array();
            auto source=sources[0].toObject();
            if(loss==1){
                require(QDir(fixture.paths.sourceDir(fixture.source.key)).removeRecursively(),"remove whole source fixture");
            }else{
                require(QFile::remove(fixture.paths.playlistItemsFile(fixture.source.key)),"remove items fixture");
                if(loss==2||loss==3||loss==4||loss==5){
                    source["last_scan_at"]="";source["last_scan_status"]="never";sources[0]=source;
                    write(fixture.paths.sourcesFile(),QJsonDocument(sources).toJson());
                }
                if(loss==2||loss==4)require(QFile::remove(fixture.paths.playlistFile(fixture.source.key)),"remove metadata fixture");
                if(loss==3)require(QFile::remove(fixture.paths.playlistHistoryFile(fixture.source.key)),"remove history fixture");
                if(loss==4)write(fixture.paths.playlistHistoryFile(fixture.source.key),{}); // Empty history still records ownership.
                if(loss==5){
                    require(QFile::remove(fixture.paths.playlistHistoryFile(fixture.source.key)),"remove history for corrupt meta");
                    write(fixture.paths.playlistFile(fixture.source.key),"not-json");
                }
                if(loss==6){
                    // A completed empty playlist also owns an authoritative empty array.
                    fixture.snapshot.items.clear();write(fixture.paths.playlistItemsFile(fixture.source.key),originalItems);
                    require(fixture.store.reconcile(fixture.source,fixture.snapshot).committed,"empty managed scan setup");
                    require(QFile::remove(fixture.paths.playlistItemsFile(fixture.source.key)),"remove empty managed items");
                }
            }
            const auto before=snapshot(fixture.paths.root());
            reason.clear();fixture.store.loadPlaylistItems(fixture.source.key,&reason);
            require(!reason.isEmpty(),QString("lost playlist state accepted by load, case %1").arg(loss));
            require(snapshot(fixture.paths.root())==before,"load changed surviving evidence");
            reason.clear();require(!fixture.store.initialize(&reason),QString("restart accepted lost state, case %1").arg(loss));
            require(snapshot(fixture.paths.root())==before,"restart changed surviving evidence");
            fixture.snapshot.items.resize(1);
            const auto result=fixture.store.reconcile(fixture.source,fixture.snapshot);
            require(!result.committed&&!result.error.isEmpty(),"A-only scan silently forgot B");
            require(snapshot(fixture.paths.root())==before,"reconciliation rewrote surviving evidence");
            require(!fixture.store.savePlaylistItems(fixture.source.key,{},&reason),"direct save normalized missing state");
            require(snapshot(fixture.paths.root())==before,"save rewrote surviving evidence");
            if(loss==0){
                write(fixture.paths.playlistItemsFile(fixture.source.key),originalItems);
                require(fixture.store.initialize(&reason),"restored original items could not reopen: "+reason);
                const auto repaired=fixture.store.reconcile(fixture.source,fixture.snapshot);
                require(repaired.committed&&repaired.removed==1,"restored evidence did not preserve B as removed");
            }
        }
        QTextStream(stdout)<<"missing-playlist: 4 first-admission modes and 7 loss/evidence cases passed\n";
        return true;
    }catch(const std::exception& ex){if(error)*error=QString::fromUtf8(ex.what());return false;}
}
}
#endif
