#include "../src/archive/archivecore.h"
#include "../src/archive/archivesafety.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTextStream>
#include <QFileInfo>
#include <functional>
#include <stdexcept>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

using namespace archive;
static void require(bool ok,const QString& message){if(!ok)throw std::runtime_error(message.toStdString());}
static QString testTempTemplate(){return QDir(qEnvironmentVariable("ARCHIVE_TEST_TMP",QDir::tempPath())).filePath("archive-hardening-XXXXXX");}
static void put(const QString& path,const QByteArray& bytes){QDir().mkpath(QFileInfo(path).absolutePath());QFile f(path);require(f.open(QIODevice::WriteOnly),"write "+path);require(f.write(bytes)==bytes.size(),"short write");}
static QByteArray get(const QString& path){QFile f(path);require(f.open(QIODevice::ReadOnly),"read "+path);return f.readAll();}
static quint32 lastLinkError=0;
static bool makeDirectoryLink(const QString& target,const QString& link)
{
#ifdef Q_OS_WIN
 const auto created=CreateSymbolicLinkW(reinterpret_cast<LPCWSTR>(link.utf16()),reinterpret_cast<LPCWSTR>(target.utf16()),0x3);
 lastLinkError=created?0:static_cast<quint32>(GetLastError());return created!=0;
#else
 return QFile::link(target,link);
#endif
}
struct Fixture {
 QTemporaryDir tmp{testTempTemplate()}; Paths paths{tmp.path()}; Store store{paths}; ActivityLogger logger{paths};
 RuntimeConfig config{tmp.path(),QCoreApplication::applicationDirPath()};
 Source source; PlaylistItem item;
 Fixture(){QString e;require(tmp.isValid()&&store.initialize(&e),e);source.key="PLAUDIT";source.url="https://www.youtube.com/playlist?list=PLAUDIT";source.title="Audit";item.providerId="abc123DEF45";item.itemKey="youtube:"+item.providerId;item.position=1;item.title="Historical title";item.availability="public";item.url="https://www.youtube.com/watch?v="+item.providerId;Snapshot s;s.sourceKey=source.key;s.complete=true;s.items={item};require(store.reconcile(source,s).committed,"fixture reconcile");}
 QString package(const QJsonObject& changes={}){const auto dir=QDir(paths.importsPending()).filePath("audit-package");QJsonObject o{{"schema_version",1},{"package_id","audit-package"},{"target",QJsonObject{{"item_key",item.itemKey},{"youtube_id",item.providerId}}},{"provenance",QJsonObject{{"method","old_local_backup"},{"confidence","high"}}},{"metadata",QJsonObject{{"canonical_title","Recovered title"}}}};for(auto i=changes.begin();i!=changes.end();++i)o[i.key()]=i.value();put(QDir(dir).filePath("manifest.json"),QJsonDocument(o).toJson());return dir;}
};
int main(int argc,char** argv){QCoreApplication app(argc,argv);if(argc!=2)return 2;const QString name=app.arguments().at(1);try{
#ifdef Q_OS_WIN
 const QStringList linkCases{"root-symlink","parent-symlink","linked-state","linked-media","linked-package-file"};
 if(linkCases.contains(name)){
  QTemporaryDir probe(testTempTemplate());const auto target=QDir(probe.path()).filePath("target");const auto link=QDir(probe.path()).filePath("link");QDir().mkpath(target);
  if(!makeDirectoryLink(target,link)&&lastLinkError==1314u){QTextStream(stdout)<<"SKIP: symbolic-link privilege unavailable\n";return 77;}
 }
#endif
 if(name=="sealed-tool-policy"){
  QTemporaryDir appRoot(testTempTemplate()),pathRoot(testTempTemplate());
  require(appRoot.isValid()&&pathRoot.isValid(),"temporary tool roots");
#ifdef Q_OS_WIN
  const auto toolName=QString("yt-dlp.exe");
#else
  const auto toolName=QString("yt-dlp");
#endif
  const auto fake=QDir(pathRoot.path()).filePath(toolName);
  require(QFile::copy(QCoreApplication::applicationFilePath(),fake),"copy PATH substitute");
  QFile::setPermissions(fake,QFile::permissions(fake)|QFileDevice::ExeOwner|QFileDevice::ExeUser|QFileDevice::ExeGroup|QFileDevice::ExeOther);
  const auto oldPath=qgetenv("PATH");
  qputenv("PATH",(pathRoot.path()+QDir::listSeparator()+QString::fromLocal8Bit(oldPath)).toLocal8Bit());
  RuntimeConfig sealed{appRoot.path(),appRoot.path()};
  require(ToolResolver(sealed).ytDlp().isEmpty(),"sealed mode accepted a PATH substitute");
  RuntimeConfig development{appRoot.path(),appRoot.path(),true};
  const auto resolved=ToolResolver(development).ytDlp();
  require(!resolved.isEmpty()&&QFileInfo(resolved).fileName().compare(toolName,Qt::CaseInsensitive)==0,
          "explicit development mode could not opt into system tools");
  qputenv("PATH",oldPath);
  return 0;
 }
 if(name=="discovery-shape"){Source s;auto r=PlaylistDiscovery::parse(s,"{}","",0);require(!r.complete,"non-playlist JSON must not be complete");}

 else if(name=="discovery-root-identity"){
  Fixture f;const QJsonArray noEntries;
  QVector<QJsonObject> suspect{{{"entries",noEntries}},{{"id",""},{"entries",noEntries}},
   {{"id",1},{"entries",noEntries}},{{"id",QJsonValue::Null},{"entries",noEntries}},
   {{"id","PLWRONG"},{"entries",noEntries}}};
  for(const auto& object:suspect){
   const auto snapshot=PlaylistDiscovery::parse(f.source,QJsonDocument(object).toJson(),{},0);
   require(!snapshot.complete,"unbound provider output authorized removals");
   require(!snapshot.error.isEmpty(),"unbound identity lacked diagnostic");
   require(f.store.reconcile(f.source,snapshot).committed,"partial observation preservation failed");
   const auto rows=f.store.loadPlaylistItems(f.source.key);
   require(rows.size()==1&&rows.first().membership=="active","unbound output removed historical occurrence");
  }
  const QJsonArray unboundEntries{QJsonObject{{"id","xyz987QWE65"},{"title","Wrong source"}}};
  for(auto object:QVector<QJsonObject>{{{"entries",unboundEntries}},{{"id","PLWRONG"},{"entries",unboundEntries}}}){
   const auto snapshot=PlaylistDiscovery::parse(f.source,QJsonDocument(object).toJson(),{},0);
   require(!snapshot.complete&&snapshot.items.isEmpty(),"unbound entries were attributed to the requested source");
   require(f.store.reconcile(f.source,snapshot).committed,"refused observations could not preserve history");
   require(f.store.loadPlaylistItems(f.source.key).size()==1,"wrong-source membership was inserted");
  }
  const auto valid=PlaylistDiscovery::parse(f.source,QJsonDocument(QJsonObject{{"id",f.source.key},{"entries",noEntries}}).toJson(),{},0);
  require(valid.complete,"exact provider identity rejected a complete empty snapshot");
  require(f.store.reconcile(f.source,valid).committed,"valid complete snapshot not committed");
  require(f.store.loadPlaylistItems(f.source.key).first().membership=="removed","legitimate removal inference was disabled");
 }
 else if(name=="discovery-explicit-index"){
  Fixture f;
  const QVector<QJsonValue> bad{QString("1"),QJsonValue::Null,true,QJsonObject{{"index",1}},
      QJsonArray{1},0,-1,1.5,2147483648.0};
  for(const auto& index:bad){
   const QJsonObject entry{{"id","xyz987QWE65"},{"title","Observed"},{"playlist_index",index}};
   const auto snapshot=PlaylistDiscovery::parse(f.source,QJsonDocument(QJsonObject{{"id",f.source.key},{"entries",QJsonArray{entry}}}).toJson(),{},0);
   require(!snapshot.complete,"malformed explicit position was coerced into complete discovery");
   require(f.store.reconcile(f.source,snapshot).committed,"partial malformed-index observation failed");
   const auto rows=f.store.loadPlaylistItems(f.source.key);
   bool kept=false;for(const auto& row:rows)if(row.itemKey==f.item.itemKey)kept=row.membership=="active";
   require(kept,"malformed explicit position authorized removal");
  }
 }
 else if(name=="discovery-position-uniqueness"){
  Fixture f;
  const QJsonObject first{{"id","xyz987QWE65"},{"title","One"},{"playlist_index",1}};
  const QJsonObject second{{"id","AAA111bbb22"},{"title","Two"},{"playlist_index",1}};
  const auto duplicate=PlaylistDiscovery::parse(f.source,QJsonDocument(QJsonObject{{"id",f.source.key},{"entries",QJsonArray{first,second}}}).toJson(),{},0);
  require(!duplicate.complete,"duplicate explicit positions authorized removals");
  require(f.store.reconcile(f.source,duplicate).committed,"partial duplicate observation failed");
  bool kept=false;for(const auto& row:f.store.loadPlaylistItems(f.source.key))if(row.itemKey==f.item.itemKey)kept=row.membership=="active";
  require(kept,"contradictory positions removed history");
  auto absentFirst=first,absentSecond=second;absentFirst.remove("playlist_index");absentSecond.remove("playlist_index");
  const auto ordered=PlaylistDiscovery::parse(f.source,QJsonDocument(QJsonObject{{"id",f.source.key},{"entries",QJsonArray{absentFirst,absentSecond}}}).toJson(),{},0);
  require(ordered.complete&&ordered.items.size()==2,"supported absent-index fallback rejected");
  require(ordered.items[0].position==1&&ordered.items[1].position==2,"array-order fallback not deterministic");
  auto mixedFirst=first;mixedFirst["playlist_index"]=2;
  const auto mixed=PlaylistDiscovery::parse(f.source,QJsonDocument(QJsonObject{{"id",f.source.key},{"entries",QJsonArray{mixedFirst,absentSecond}}}).toJson(),{},0);
  require(!mixed.complete,"explicit and inferred position conflict authorized removals");
  auto repeated=absentFirst;
  const auto repeat=PlaylistDiscovery::parse(f.source,QJsonDocument(QJsonObject{{"id",f.source.key},{"entries",QJsonArray{repeated,repeated}}}).toJson(),{},0);
  require(repeat.complete&&repeat.items.size()==2,"legitimate repeated video occurrences disabled");
 }
 else if(name=="discovery-null"){Source s;auto r=PlaylistDiscovery::parse(s,R"({"entries":[null]})","",0);require(!r.complete,"null entries must prevent removal inference");}
 else if(name=="discovery-count"){Source s;auto r=PlaylistDiscovery::parse(s,R"({"entries":[],"playlist_count":3})","",0);require(!r.complete,"truncated playlist must be partial");}
 else if(name=="discovery-error"){Source s;auto r=PlaylistDiscovery::parse(s,R"({"entries":[]})","ERROR: failed to fetch page",0);require(!r.complete,"ignore-errors cannot authorize removals");}
  else if(name=="portable-paths"){Fixture f;for(const auto& p:QStringList{"C:escape","Video/../State/items.json","Video\\..\\escape","Video/NUL.mp4","Video/x:stream","Video/trailing. ","Video//x","Video/./x"})require(!f.paths.isSafeRelative(p),"unsafe portable path accepted: "+p);require(f.paths.isSafeRelative("Video/a file [abc123DEF45].mp4"),"normal path rejected");}
  else if(name=="reserved-device-variants"){Fixture f;for(const auto& p:QStringList{"Video/CON.txt","Video/com1.mp4","Video/COM¹.mp4","Video/COM².mp4","Video/COM³.mp4","Video/LPT¹.txt"})require(!f.paths.isSafeRelative(p),"reserved device name accepted: "+p);}
  else if(name=="reparse-policy"){require(detail::isPermittedCloudFilesTag(0x9000601Au),"Cloud Files family rejected");require(!detail::isPermittedCloudFilesTag(0xA000000Cu),"symbolic-link tag permitted");require(!detail::isPermittedCloudFilesTag(0xA0000003u),"junction tag permitted");require(!detail::isPermittedCloudFilesTag(0x12345678u),"unknown tag permitted");}
  else if(name=="root-symlink"){QTemporaryDir base,target;require(base.isValid()&&target.isValid(),"temporary roots");const auto link=QDir(base.path()).filePath("root-link");require(makeDirectoryLink(target.path(),link),"create root symbolic link");Paths p(link);QString e;require(!p.ensureLayout(&e),"symbolic-link Archive Root accepted");}
  else if(name=="parent-symlink"){QTemporaryDir base,target;require(base.isValid()&&target.isValid(),"temporary roots");const auto link=QDir(base.path()).filePath("parent-link");require(makeDirectoryLink(target.path(),link),"create parent symbolic link");Paths p(QDir(link).filePath("archive"));QString e;require(!p.ensureLayout(&e),"symbolic-link Archive Root parent accepted");}
  else if(name=="linked-state"){Fixture f;const auto outside=QDir(f.tmp.path()).filePath("outside-state");QDir().mkpath(outside);QDir(f.paths.state()).removeRecursively();require(makeDirectoryLink(outside,f.paths.state()),"create linked State directory");QString e;require(!f.paths.ensureLayout(&e),"linked State directory accepted");}
  else if(name=="linked-media"){Fixture f;const auto outside=QDir(f.tmp.path()).filePath("outside-video");QDir().mkpath(outside);QDir(f.paths.video()).removeRecursively();require(makeDirectoryLink(outside,f.paths.video()),"create linked Video directory");QString e;require(!f.paths.ensureLayout(&e),"linked Video directory accepted");}
  else if(name=="placeholder-position"){require(canonicalKey({},"PLAUDIT",1,"Unresolved")==canonicalKey({},"PLAUDIT",99,"Unresolved"),"placeholder identity depends on mutable position");}
  else if(name=="placeholder-reorder"){
   Fixture f;Source source;source.key="PLUNKNOWN";source.url="https://www.youtube.com/playlist?list=PLUNKNOWN";source.title="Unknown";Snapshot first;first.sourceKey=source.key;first.complete=true;
   for(int position:QList<int>{3,4}){PlaylistItem item;item.title="Same unresolved title";item.url="https://example.invalid/unresolved";item.position=position;item.itemKey=canonicalKey({},source.key,position,item.title);item.availability="public";first.items.append(item);}
   require(f.store.reconcile(source,first).committed,"initial unresolved reconciliation failed");QString e;const auto before=f.store.loadPlaylistItems(source.key,&e);require(e.isEmpty()&&before.size()==2,"initial unresolved occurrences missing");
   Snapshot reordered;reordered.sourceKey=source.key;reordered.complete=true;
   for(int position:QList<int>{1,2}){PlaylistItem item;item.title="Same unresolved title";item.url="https://example.invalid/unresolved";item.position=position;item.itemKey=canonicalKey({},source.key,position,item.title);item.availability="public";reordered.items.append(item);}
   require(f.store.reconcile(source,reordered).committed,"reordered unresolved reconciliation failed");const auto after=f.store.loadPlaylistItems(source.key,&e);require(e.isEmpty()&&after.size()==2,"reordered unresolved occurrences missing");
   QSet<QString> beforeKeys,afterKeys;for(const auto& item:before)beforeKeys.insert(item.itemKey);for(const auto& item:after)afterKeys.insert(item.itemKey);require(beforeKeys==afterKeys,"unresolved reorder changed canonical identities");
  }
  else if(name=="placeholder-metadata-drift"){
   const auto resolved=[](const QString& id,int position){
    PlaylistItem item;item.providerId=id;item.itemKey="youtube:"+id;item.title="Resolved "+id;item.url="https://www.youtube.com/watch?v="+id;item.position=position;item.availability="public";return item;
   };

   // Provider placeholder labels and URLs may drift while the same historical
   // occurrence remains bracketed by stable resolved neighbors.
   Fixture f;Source source;source.key="PLDRIFT";source.url="https://www.youtube.com/playlist?list=PLDRIFT";source.title="Drift";
   Snapshot first;first.sourceKey=source.key;first.complete=true;first.scannedAt="2026-01-01T00:00:00.000Z";
   first.items.append(resolved("AAA111bbb22",1));
   PlaylistItem old;old.title="[Private video]";old.url="https://example.invalid/private-slot";old.position=2;old.itemKey=canonicalKey({},source.key,2,old.title);old.availability="private";first.items.append(old);
   first.items.append(resolved("BBB222ccc33",3));
   require(f.store.reconcile(source,first).committed,"initial placeholder drift setup failed");

   QString e;const auto before=f.store.loadPlaylistItems(source.key,&e);require(e.isEmpty(),"load drift baseline");
   PlaylistItem priorPlaceholder;bool foundPrior=false;
   for(const auto& row:before)if(row.providerId.isEmpty()&&row.position==2){priorPlaceholder=row;foundPrior=true;break;}
   require(foundPrior,"baseline placeholder missing");

   Snapshot second;second.sourceKey=source.key;second.complete=true;second.scannedAt="2026-01-02T00:00:00.000Z";
   second.items.append(resolved("AAA111bbb22",1));
   PlaylistItem drifted;drifted.title="[Deleted video]";drifted.url="";drifted.position=2;drifted.itemKey=canonicalKey({},source.key,2,drifted.title);drifted.availability="deleted";second.items.append(drifted);
   second.items.append(resolved("BBB222ccc33",3));
   const auto driftSummary=f.store.reconcile(source,second);require(driftSummary.committed,"metadata-drift reconciliation failed: "+driftSummary.error);

   const auto after=f.store.loadPlaylistItems(source.key,&e);require(e.isEmpty(),"load drift result");
   PlaylistItem continued;bool foundContinued=false;
   for(const auto& row:after)if(row.providerId.isEmpty()&&row.membership=="active"&&row.position==2){continued=row;foundContinued=true;break;}
   require(foundContinued,"drifted placeholder missing after reconcile");
   require(continued.itemKey==priorPlaceholder.itemKey,"metadata drift split canonical placeholder identity");
   require(continued.entryKey==priorPlaceholder.entryKey,"metadata drift split occurrence identity");
   require(continued.firstSeen==priorPlaceholder.firstSeen,"metadata drift reset first-seen evidence");
   require(continued.title=="[Deleted video]","drifted observation metadata was not refreshed");

   // Two plausible historical placeholders without position/neighbor evidence
   // are ambiguous and must not be guessed.
   Fixture ambiguous;Source as;as.key="PLDRIFTAMB";as.url="https://www.youtube.com/playlist?list=PLDRIFTAMB";as.title="Ambiguous drift";
   Snapshot a1;a1.sourceKey=as.key;a1.complete=true;
   for(int position:QList<int>{1,2}){PlaylistItem p;p.title="[Private video]";p.url="";p.position=position;p.itemKey=canonicalKey({},as.key,position,p.title);p.availability="private";a1.items.append(p);}
   require(ambiguous.store.reconcile(as,a1).committed,"ambiguous drift setup failed");
   const auto ambiguousBefore=ambiguous.store.loadPlaylistItems(as.key,&e);require(e.isEmpty()&&ambiguousBefore.size()==2,"ambiguous baseline missing");
   QSet<QString> oldEntries;for(const auto& row:ambiguousBefore)oldEntries.insert(row.entryKey);
   Snapshot a2;a2.sourceKey=as.key;a2.complete=true;PlaylistItem ap;ap.title="[Deleted video]";ap.url="";ap.position=99;ap.itemKey=canonicalKey({},as.key,99,ap.title);ap.availability="deleted";a2.items.append(ap);
   require(ambiguous.store.reconcile(as,a2).committed,"ambiguous drift reconcile failed");
   const auto ambiguousAfter=ambiguous.store.loadPlaylistItems(as.key,&e);require(e.isEmpty(),"load ambiguous result");
   bool sawFresh=false;for(const auto& row:ambiguousAfter)if(row.membership=="active"&&row.position==99){sawFresh=true;require(!oldEntries.contains(row.entryKey),"ambiguous drift guessed an old occurrence");}
   require(sawFresh,"ambiguous drift did not retain fresh observation");

   // Same position and neighbors are not enough when the new unresolved item
   // has unrelated descriptive evidence.
   Fixture replacement;Source rs;rs.key="PLDRIFTNEW";rs.url="https://www.youtube.com/playlist?list=PLDRIFTNEW";rs.title="Replacement";
   Snapshot r1;r1.sourceKey=rs.key;r1.complete=true;r1.items.append(resolved("CCC333ddd44",1));
   PlaylistItem rp;rp.title="Old unresolved";rp.url="https://example.invalid/old";rp.position=2;rp.itemKey=canonicalKey({},rs.key,2,rp.title);rp.availability="unavailable";r1.items.append(rp);
   r1.items.append(resolved("DDD444eee55",3));require(replacement.store.reconcile(rs,r1).committed,"replacement setup failed");
   const auto replacementBefore=replacement.store.loadPlaylistItems(rs.key,&e);require(e.isEmpty(),"load replacement baseline");
   QString oldEntry;for(const auto& row:replacementBefore)if(row.providerId.isEmpty())oldEntry=row.entryKey;require(!oldEntry.isEmpty(),"replacement old placeholder missing");
   Snapshot r2;r2.sourceKey=rs.key;r2.complete=true;r2.items.append(resolved("CCC333ddd44",1));
   PlaylistItem np;np.title="Completely new unresolved";np.url="https://example.invalid/new";np.position=2;np.itemKey=canonicalKey({},rs.key,2,np.title);np.availability="unavailable";r2.items.append(np);
   r2.items.append(resolved("DDD444eee55",3));require(replacement.store.reconcile(rs,r2).committed,"replacement reconcile failed");
   const auto replacementAfter=replacement.store.loadPlaylistItems(rs.key,&e);require(e.isEmpty(),"load replacement result");
   bool sawReplacement=false;for(const auto& row:replacementAfter)if(row.membership=="active"&&row.position==2&&row.providerId.isEmpty()){sawReplacement=true;require(row.entryKey!=oldEntry,"unrelated replacement was merged into old placeholder");}
   require(sawReplacement,"new unresolved replacement missing");
  }

  else if(name=="placeholder-promotion"){
   Fixture f;Source source;source.key="PLPROMOTE";source.url="https://www.youtube.com/playlist?list=PLPROMOTE";source.title="Promotion";
   Snapshot first;first.sourceKey=source.key;first.complete=true;first.scannedAt="2026-01-01T00:00:00.000Z";
   for(int position:QList<int>{1,2}){PlaylistItem item;item.title="Resolvable title";item.url="https://example.invalid/stable";item.position=position;item.itemKey=canonicalKey({},source.key,position,item.title);item.availability="public";first.items.append(item);}
   require(f.store.reconcile(source,first).committed,"initial placeholder reconciliation failed");
   QString e;auto beforeRows=f.store.loadPlaylistItems(source.key,&e);require(e.isEmpty()&&beforeRows.size()==2,"initial placeholder rows missing");
   QHash<int,PlaylistItem> byPosition;for(const auto& row:beforeRows)byPosition[row.position]=row;
   const auto promotedOldKey=byPosition.value(2).itemKey;const auto promotedEntry=byPosition.value(2).entryKey;const auto firstSeen=byPosition.value(2).firstSeen;

   auto canonical=f.store.loadCanonicalItems(&e);require(e.isEmpty(),"load canonical before promotion");
   bool attached=false;for(auto& item:canonical)if(item.key==promotedOldKey){item.video.state="complete";item.video.path="Video/preserved.mp4";item.video.origin="test";item.video.verifiedAt="2026-01-01T00:00:01.000Z";item.recoveryStatus="unrecovered";attached=true;}
   require(attached&&f.store.saveCanonicalItems(canonical,&e),"attach placeholder representation: "+e);

   Snapshot second;second.sourceKey=source.key;second.complete=true;second.scannedAt="2026-01-02T00:00:00.000Z";
   PlaylistItem unresolved;unresolved.title="Resolvable title";unresolved.url="https://example.invalid/stable";unresolved.position=1;unresolved.itemKey=canonicalKey({},source.key,1,unresolved.title);unresolved.availability="public";second.items.append(unresolved);
   PlaylistItem resolved;resolved.providerId="AAA111bbb22";resolved.title="Resolvable title";resolved.url="https://example.invalid/stable";resolved.position=2;resolved.itemKey=canonicalKey(resolved.providerId,source.key,2,resolved.title);resolved.availability="public";second.items.append(resolved);
   const auto promoted=f.store.reconcile(source,second);require(promoted.committed,"resolved placeholder promotion failed: "+promoted.error);

   const auto afterRows=f.store.loadPlaylistItems(source.key,&e);require(e.isEmpty()&&afterRows.size()==2,"promoted playlist row count changed");
   bool sawResolved=false,sawUnresolved=false;for(const auto& row:afterRows){
    if(row.providerId=="AAA111bbb22"){sawResolved=true;require(row.itemKey=="youtube:AAA111bbb22","resolved canonical key not promoted");require(row.entryKey==promotedEntry,"occurrence entry identity changed");require(row.firstSeen==firstSeen,"promotion reset first-seen evidence");}
    else {sawUnresolved=true;require(row.position==1&&row.membership=="active","unresolved sibling was not preserved");}
   }
   require(sawResolved&&sawUnresolved,"duplicate placeholder promotion lost an occurrence");

   const auto afterCanonical=f.store.loadCanonicalItems(&e);require(e.isEmpty(),"load canonical after promotion");
   bool sawNew=false,sawOld=false;for(const auto& item:afterCanonical){if(item.key=="youtube:AAA111bbb22"){sawNew=true;require(item.video.state=="complete"&&item.video.path=="Video/preserved.mp4","representation state detached during promotion");require(item.recoveryStatus=="unrecovered","recovery state detached during promotion");}if(item.key==promotedOldKey)sawOld=true;}
   require(sawNew&&!sawOld,"placeholder canonical record was duplicated instead of promoted");
   const auto history=get(f.paths.playlistHistoryFile(source.key));require(history.contains("\"event\":\"identity_promoted\""),"promotion history event missing");require(history.contains(promotedOldKey.toUtf8()),"promotion history lost old identity");

   // With two unmatched candidates and no unique position match, the resolved
   // observation must not guess which placeholder owns the historical state.
   Fixture ambiguous;Source ambiguousSource=source;ambiguousSource.key="PLAMBIG";Snapshot a;a.sourceKey=ambiguousSource.key;a.complete=true;
   for(int position:QList<int>{1,2}){PlaylistItem item;item.title="Ambiguous";item.url="https://example.invalid/ambiguous";item.position=position;item.itemKey=canonicalKey({},ambiguousSource.key,position,item.title);item.availability="public";a.items.append(item);}
   require(ambiguous.store.reconcile(ambiguousSource,a).committed,"ambiguous setup failed");
   Snapshot b;b.sourceKey=ambiguousSource.key;b.complete=true;PlaylistItem r;r.providerId="CCC333ddd44";r.title="Ambiguous";r.url="https://example.invalid/ambiguous";r.position=99;r.itemKey=canonicalKey(r.providerId,ambiguousSource.key,99,r.title);r.availability="public";b.items.append(r);
   require(ambiguous.store.reconcile(ambiguousSource,b).committed,"ambiguous resolved observation failed");
   const auto ambiguousHistory=get(ambiguous.paths.playlistHistoryFile(ambiguousSource.key));require(!ambiguousHistory.contains("\"event\":\"identity_promoted\""),"ambiguous placeholders were auto-merged");
  }

 else if(name=="orphan-data"){
  // Registry loss must never turn existing media, state or recovery evidence
  // into a new empty archive. Each path is isolated and checked byte-for-byte.
  const QStringList evidence{
   "Video/kept.mp4", "Audio/kept.m4a", "Metadata/kept.json",
   "State/download-archive.txt", "State/ArchiveMode/catalog.json",
   "Temp/interrupted.part", "Video/.hidden-media",
   "State/ArchiveMode/Imports/Pending/old/manifest.json",
   "State/ArchiveMode/Imports/Accepted/old/receipt.json",
   "State/ArchiveMode/Imports/Rejected/old/manifest.json",
   "State/ArchiveMode/Logs/Activity/2000-01-01/history.jsonl",
   "ARCHIVE_AGENT.md"
  };
  for(const auto& relative:evidence){
   QTemporaryDir tmp(testTempTemplate());require(tmp.isValid(),"temporary root");
   Paths paths(tmp.path());const auto file=QDir(tmp.path()).filePath(relative);
   const QByteArray bytes="preserve this existing archive evidence\n";put(file,bytes);
   Store store(paths);QString error;
   require(!store.initialize(&error),"orphan evidence accepted as fresh: "+relative);
   require(!error.isEmpty(),"orphan refusal needs an actionable error");
   require(get(file)==bytes,"orphan evidence changed: "+relative);
   require(!QFileInfo::exists(paths.sourcesFile())&&!QFileInfo::exists(paths.itemsFile()),"empty registries were created beside "+relative);
   require(!store.initialize(&error),"repeated initialization bypassed orphan refusal");
  }
  for(const auto& relative:QStringList{"Video/old-empty-directory","Playlists/PLLOST","Metadata/old-empty-directory"}){
   QTemporaryDir tmp(testTempTemplate());require(tmp.isValid(),"temporary root");
   Paths paths(tmp.path());require(QDir().mkpath(QDir(tmp.path()).filePath(relative)),"make orphan directory");
   Store store(paths);QString error;require(!store.initialize(&error),"orphan directory accepted: "+relative);
  }
 }
 else if(name=="archive-identity"){
  QTemporaryDir tmp(testTempTemplate());require(tmp.isValid(),"temporary root");
  Paths paths(tmp.path());Store store(paths);QString error;
  // Root-level unrelated documentation is not an archive payload. Generated
  // empty layout from the existing logger/lock constructors remains admissible.
  put(QDir(tmp.path()).filePath("operator-notes.txt"),"keep notes\n");
  require(paths.ensureLayout(&error),error);
  require(store.initialize(&error),"genuinely fresh root rejected: "+error);
  const auto identity=QDir(paths.archiveState()).filePath("archive-identity.json");
  require(QFileInfo::exists(identity),"fresh archive has no durable identity");
  const auto identityBytes=get(identity);const auto object=QJsonDocument::fromJson(identityBytes).object();
  require(object.value("schema_version")==1&&!object.value("archive_id").toString().isEmpty(),"invalid identity marker");
  require(store.initialize(&error),error);require(get(identity)==identityBytes,"identity changed after restart");
  // Even an otherwise empty previously initialized archive must not reset.
  require(QFile::remove(paths.sourcesFile())&&QFile::remove(paths.itemsFile()),"remove fixture registries");
  require(!store.initialize(&error),"identified archive silently reset after both registries disappeared");
  require(get(identity)==identityBytes,"lost-registry refusal changed identity");
  require(get(QDir(tmp.path()).filePath("operator-notes.txt"))=="keep notes\n","unrelated file changed");
 }
 else if(name=="legacy-admission"){
  Fixture f;const auto identity=QDir(f.paths.archiveState()).filePath("archive-identity.json");
  if(QFileInfo::exists(identity))require(QFile::remove(identity),"remove fixture identity for legacy test");
  const auto sources=get(f.paths.sourcesFile()),items=get(f.paths.itemsFile());
  const auto playlist=get(f.paths.playlistItemsFile(f.source.key));
  const auto history=get(f.paths.playlistHistoryFile(f.source.key));QString error;
  require(f.store.initialize(&error),"valid legacy archive rejected: "+error);
  require(QFileInfo::exists(identity),"legacy archive identity migration missing");
  require(get(f.paths.sourcesFile())==sources&&get(f.paths.itemsFile())==items,"legacy registry rewritten by identity migration");
  require(get(f.paths.playlistItemsFile(f.source.key))==playlist&&get(f.paths.playlistHistoryFile(f.source.key))==history,"legacy history changed");
  const QByteArray damaged="{\"schema_version\":99,\"archive_id\":\"broken\"}\n";put(identity,damaged);
  require(!f.store.initialize(&error),"unsupported identity marker accepted");
  require(!error.isEmpty(),"unsupported marker must explain its refusal");
  require(get(identity)==damaged&&get(f.paths.itemsFile())==items,"unsupported marker or registry overwritten");
  require(QFile::remove(identity)&&QFile::remove(f.paths.sourcesFile()),"prepare incomplete registry");
  require(!f.store.initialize(&error),"one missing legacy registry accepted");
  require(get(f.paths.itemsFile())==items,"incomplete legacy state overwritten");
 }

 else if(name=="admission-journal"){
  QTemporaryDir tmp(testTempTemplate());require(tmp.isValid(),"temporary root");Paths paths(tmp.path());Store store(paths);QString error;
  require(paths.ensureLayout(&error),error);
  const QString identityRel="State/ArchiveMode/archive-identity.json";
  const auto identity=QJsonDocument(QJsonObject{{"schema_version",1},{"format","mdps-archive"},
      {"archive_id","11111111-2222-4333-8444-555555555555"},
      {"created_at","2026-01-01T00:00:00.000Z"},{"admitted_from","fresh"}}).toJson();
  QMap<QString,QByteArray> writes{{identityRel,identity},{"State/ArchiveMode/items.json","[]\n"},{"State/ArchiveMode/sources.json","[]\n"}};
  QJsonArray operations;
  for(auto i=writes.begin();i!=writes.end();++i)operations.append(QJsonObject{
      {"path",i.key()},{"before_exists",false},{"before_sha256",detail::digest({})},
      {"after_base64",QString::fromLatin1(i.value().toBase64())},{"after_sha256",detail::digest(i.value())}});
  const auto journal=QJsonDocument(QJsonObject{{"schema_version",1},{"operations",operations}}).toJson();
  put(detail::journalPath(paths.root()),journal);
  // Simulate a process dying after the first atomic replacement. Restart must
  // honor the complete durable intent, not reject or generate a new identity.
  put(QDir(paths.root()).filePath(identityRel),identity);
  require(store.initialize(&error),"interrupted initialization did not recover: "+error);
  require(get(QDir(paths.root()).filePath(identityRel))==identity,"restart changed journaled identity");
  require(get(paths.itemsFile())=="[]\n"&&get(paths.sourcesFile())=="[]\n","restart did not restore exact registry payloads");
  require(!QFileInfo::exists(detail::journalPath(paths.root())),"completed admission intent not retired");
  require(store.initialize(&error),"repeated admission recovery failed");
 }

 else if(name=="projection-commit-outcome"){
  Fixture f;const auto catalog=QDir(f.paths.sourceDir(f.source.key)).filePath("catalog.csv");
  require(QFile::remove(catalog)&&QDir().mkdir(catalog),"block generated catalog path");
  Snapshot snapshot;snapshot.sourceKey=f.source.key;snapshot.complete=true;snapshot.items={f.item};
  const auto result=f.store.reconcile(f.source,snapshot,&f.logger);
  require(result.committed,"MDPS-AUDIT2-025: projection failure hid a durable reconciliation commit");
  require(!result.projectionsCurrent&&!result.projectionWarning.isEmpty()&&result.error.isEmpty(),"commit and projection errors conflated");
  require(result.active==1&&result.observed==1,"committed counts lost after projection failure");
  const auto marker=QDir(f.paths.archiveState()).filePath("projections-dirty.json");
  require(QFileInfo::exists(marker),"projection failure lost durable rebuild intent");
  const auto items=get(f.paths.itemsFile()),sources=get(f.paths.sourcesFile());
  const auto history=get(f.paths.playlistHistoryFile(f.source.key));QString error;
  require(QDir().rmdir(catalog)&&f.store.initialize(&error),"projection-only restart repair failed: "+error);
  require(!QFileInfo::exists(marker),"successful rebuild retained dirty marker");
  require(get(f.paths.itemsFile())==items&&get(f.paths.sourcesFile())==sources&&get(f.paths.playlistHistoryFile(f.source.key))==history,
      "projection repair replayed canonical mutation");
  require(f.store.initialize(&error),"repeat projection repair failed");
 }
 else if(name=="recovery-commit-outcome"){
  Fixture f;const auto package=f.package({{"metadata",QJsonObject{{"canonical_title","Recovered metadata"}}}});
  const auto manifest=get(QDir(package).filePath("manifest.json"));
  const auto catalog=QDir(f.paths.sourceDir(f.source.key)).filePath("catalog.csv");
  require(QFile::remove(catalog)&&QDir().mkdir(catalog),"block generated catalog path");
  RecoveryImporter importer(f.config,f.store,f.logger);QStringList failures,warnings;
  const auto accepted=importer.ingestPending(&failures,{},&warnings);
  require(accepted==1,"MDPS-AUDIT2-025: accepted recovery counted as pending after projection failure");
  require(failures.isEmpty(),"committed recovery reported a retryable admission failure");
  require(warnings.size()==1&&warnings.first().contains("State committed"),"committed projection warning lost");
  const auto acceptedDir=QDir(f.paths.importsAccepted()).filePath(QFileInfo(package).fileName());
  require(!QFileInfo::exists(package)&&QFileInfo(acceptedDir).isDir(),"accepted package moved to wrong state");
  require(get(QDir(acceptedDir).filePath("manifest.json"))==manifest,"accepted evidence mutated");
  const auto receipt=get(QDir(acceptedDir).filePath("receipt.json"));
  const auto items=get(f.paths.itemsFile()),history=get(f.paths.playlistHistoryFile(f.source.key));
  QString error;require(QDir().rmdir(catalog)&&f.store.initialize(&error),"recovery projection repair failed: "+error);
  require(importer.ingestPending(&failures)==0&&failures.isEmpty(),"already accepted package was retried");
  require(get(f.paths.itemsFile())==items&&get(f.paths.playlistHistoryFile(f.source.key))==history&&get(QDir(acceptedDir).filePath("receipt.json"))==receipt,
      "projection-only retry changed committed recovery evidence");
 }
 else if(name=="source-traversal"){Fixture f;Source s=f.source;s.key="..";Snapshot snap;snap.sourceKey=s.key;snap.complete=true;require(!f.store.reconcile(s,snap).committed,"unsafe source key must fail");}
 else if(name=="state-shape"){Fixture f;put(f.paths.itemsFile(),"{\"unexpected\":true}");QString e;f.store.loadCanonicalItems(&e);require(!e.isEmpty(),"wrong JSON root must be an error");const auto before=get(f.paths.itemsFile());Snapshot s;s.sourceKey=f.source.key;s.items={f.item};require(!f.store.reconcile(f.source,s).committed,"corrupt store must block writes");require(get(f.paths.itemsFile())==before,"corrupt state was overwritten");}
 else if(name=="source-state-corruption"){Fixture f;put(f.paths.sourcesFile(),"not json");Snapshot s;s.sourceKey=f.source.key;s.items={f.item};require(!f.store.reconcile(f.source,s).committed,"corrupt sources must block reconciliation");require(get(f.paths.sourcesFile())=="not json","source registry destroyed");}
 else if(name=="snapshot-identity"){Fixture f;Snapshot s;s.sourceKey="PLWRONG";s.complete=true;require(!f.store.reconcile(f.source,s).committed,"cross-source snapshot accepted");}
 else if(name=="import-identity"){Fixture f;const auto p=f.package({{"target",QJsonObject{{"item_key",f.item.itemKey},{"youtube_id","xyz987QWE65"}}}});RecoveryImporter i(f.config,f.store,f.logger);require(!i.validate(p).ok,"conflicting identities accepted");}
 else if(name=="import-id-only"){Fixture f;const auto p=f.package({{"target",QJsonObject{{"youtube_id",f.item.providerId}}}});RecoveryImporter i(f.config,f.store,f.logger);require(i.validate(p).ok,"documented youtube_id-only target rejected");}
 else if(name=="import-schema"){
   Fixture f;RecoveryImporter i(f.config,f.store,f.logger);
   const auto unsafe=f.package({{"package_id","../unsafe"},{"provenance",QJsonObject{{"method","old_local_backup"},{"confidence","certain"}}}});
   require(!i.validate(unsafe).ok,"unsafe package/schema accepted");

   const auto wrongType=f.package({{"target",QJsonObject{{"item_key",17}}}});
   require(!i.validate(wrongType).ok,"numeric item_key accepted despite schema");

   const auto missingTarget=f.package({{"target",QJsonObject{}}});
   require(!i.validate(missingTarget).ok,"missing target identity accepted");

   const auto originalCanonical=f.store.loadCanonicalItems();
   require(originalCanonical.size()==1,"fixture canonical state");
   auto placeholder=originalCanonical.first();placeholder.key="placeholder:PLAUDIT:legacy";placeholder.providerId.clear();
   require(f.store.saveCanonicalItems(QVector<CanonicalItem>{placeholder}),"placeholder fixture state");
   const auto placeholderTarget=f.package({{"target",QJsonObject{{"item_key",placeholder.key}}}});
   require(!i.validate(placeholderTarget).ok,"schema-invalid placeholder item_key accepted");

   require(f.store.saveCanonicalItems(originalCanonical),"restore canonical fixture");
   const auto canonicalTarget=f.package({{"target",QJsonObject{{"item_key",f.item.itemKey}}}});
   require(i.validate(canonicalTarget).ok,"schema-valid canonical item_key rejected");

   const auto idOnly=f.package({{"target",QJsonObject{{"youtube_id",f.item.providerId}}}});
   require(i.validate(idOnly).ok,"schema-valid youtube_id-only target rejected");
  }
 else if(name=="partial-recovery-status"){Fixture f;f.item.availability="deleted";Snapshot s;s.sourceKey=f.source.key;s.items={f.item};require(f.store.reconcile(f.source,s).committed,"deleted reconcile");Representation r;r.state="complete";r.path="Video/test.mp4";require(f.store.updateRepresentation(f.item.itemKey,"video",r),"update");require(f.store.loadCanonicalItems().first().recoveryStatus!="not_required","audio still missing but recovery cleared");}
 else if(name=="nested-redaction"){Fixture f;f.logger.event("INFO","test","redaction",{{"nested",QJsonObject{{"token","do-not-leak"}}}});const auto day=QDir(f.paths.activityLogs()).entryList(QDir::Dirs|QDir::NoDotAndDotDot).last();const auto dir=QDir(f.paths.activityLogs()).filePath(day);const auto file=QDir(dir).entryList(QDir::Files).last();require(!get(QDir(dir).filePath(file)).contains("do-not-leak"),"nested credential leaked");}
 else if(name=="deleted-title"){Fixture f;auto p=f.item;p.title="[Deleted video]";p.availability="deleted";Snapshot s;s.sourceKey=f.source.key;s.items={p};require(f.store.reconcile(f.source,s).committed,"deleted reconcile");require(get(QDir(f.paths.sourceDir(f.source.key)).filePath("missing.csv")).contains("Historical title"),"recovery report lost known title");}
  else if(name=="linked-package-file"){
   Fixture f;QTemporaryDir outside;const auto p=f.package({{"representations",QJsonObject{{"video",QJsonObject{{"file","linked.mp4"}}}}}});
#ifdef Q_OS_WIN
   const auto target=QDir::toNativeSeparators(outside.filePath("secret.mp4"));
   const auto link=QDir::toNativeSeparators(QDir(p).filePath("linked.mp4"));
   const auto created=CreateSymbolicLinkW(reinterpret_cast<LPCWSTR>(link.utf16()),reinterpret_cast<LPCWSTR>(target.utf16()),0x2);
   const auto error=GetLastError();
   require(created!=0,QString("make symlink error=%1 target=%2 link=%3").arg(error).arg(target).arg(link));
   require(QFileInfo(link).isSymLink(),"package-side symlink missing");
#else
   require(QFile::link(outside.filePath("secret.mp4"),QDir(p).filePath("linked.mp4")),"make symlink");
#endif
   RecoveryImporter i(f.config,f.store,f.logger);require(!i.validate(p).ok,"symlink escaped package boundary");
  }
 else {QTextStream(stderr)<<"Unknown case\n";return 2;}
 QTextStream(stdout)<<name<<": PASS\n";return 0;
 }catch(const std::exception& e){QTextStream(stderr)<<name<<": FAIL: "<<e.what()<<"\n";return 1;}}
