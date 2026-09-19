#include "archivecore.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>

namespace {
QString firstLine(QString value){return value.trimmed().section('\n',0,0).trimmed();}
bool checkTool(QTextStream& out,QTextStream& error,const QString& name,const QString& path,const QStringList& args){
    if(path.isEmpty()){error<<name<<"=MISSING\n";return false;}
    QProcess p;p.start(path,args);p.closeWriteChannel();
    if(!p.waitForStarted(10000)){error<<name<<"=FAILED_TO_START "<<p.errorString()<<"\n";return false;}
    if(!p.waitForFinished(30000)){p.kill();p.waitForFinished(5000);error<<name<<"=TIMEOUT\n";return false;}
    const auto stdoutText=QString::fromUtf8(p.readAllStandardOutput()),stderrText=QString::fromUtf8(p.readAllStandardError());
    if(p.exitStatus()!=QProcess::NormalExit||p.exitCode()!=0){error<<name<<"=FAILED "<<firstLine(stderrText)<<"\n";return false;}
    out<<name<<"=OK path="<<QDir::toNativeSeparators(path)<<" version="<<firstLine(stdoutText.isEmpty()?stderrText:stdoutText)<<"\n";return true;
}
void usage(QTextStream& out){out<<"Usage:\n"
    "  archive-cli preflight <archive-root>\n"
    "  archive-cli validate <archive-root> <package-dir>\n"
    "  archive-cli ingest-pending <archive-root>\n"
    "  archive-cli rebuild-projections <archive-root>\n"
    "  archive-cli scan <archive-root> <youtube-playlist-url> [display-name]\n"
    "  archive-cli playlist-binding <archive-root> <youtube-playlist-url> <youtube-video-url>\n"
    "  archive-cli sync-item <archive-root> <youtube-video-url>\n"
    "  archive-cli verify-item <archive-root> <youtube-video-url>\n";}
}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTextStream out(stdout),error(stderr);const auto args=app.arguments();
    if(args.size()<3){usage(error);return 2;}
    const auto command=args[1];
    const bool valid=(QStringList{"preflight","ingest-pending","rebuild-projections"}.contains(command)&&args.size()==3)||
        (QStringList{"validate","sync-item","verify-item"}.contains(command)&&args.size()==4)||
        (command=="scan"&&(args.size()==4||args.size()==5))||
        (command=="playlist-binding"&&args.size()==5);
    if(!valid||args[2].trimmed().isEmpty()){usage(error);return 2;}
    const auto sourceKey=(command=="scan"||command=="playlist-binding")?archive::sourceKeyFromUrl(args[3]):QString();
    const auto id=(command=="sync-item"||command=="verify-item")?archive::videoIdFromUrl(args[3]):
        command=="playlist-binding"?archive::videoIdFromUrl(args[4]):QString();
    if(((command=="scan"||command=="playlist-binding")&&sourceKey.isEmpty())||
       ((command=="sync-item"||command=="verify-item"||command=="playlist-binding")&&id.isEmpty())){
        error<<"Invalid YouTube URL or identity; no archive state was changed\n";return 2;
    }
    archive::Paths paths(args[2]);archive::RuntimeConfig config{paths.root(),QCoreApplication::applicationDirPath()};
    archive::SyncLock lock(paths);if(!lock.tryLock()){error<<lock.errorString()<<"\n";return 1;}
    archive::Store store(paths);QString stateError;
    if(!store.initialize(&stateError,lock.acquiredFreshly())){error<<"Archive initialization failed: "<<stateError<<"\n";return 1;}
    archive::ActivityLogger logger(paths);
    if(command=="preflight"){
        archive::ToolResolver tools(config);bool ok=true;
        ok=checkTool(out,error,"yt-dlp",tools.ytDlp(),{"--version"})&&ok;
        ok=checkTool(out,error,"ffmpeg",tools.ffmpeg(),{"-version"})&&ok;
        ok=checkTool(out,error,"ffprobe",tools.ffprobe(),{"-version"})&&ok;
        ok=checkTool(out,error,"deno",tools.deno(),{"--version"})&&ok;
        out<<"preflight="<<(ok?"PASS":"FAIL")<<"\n";return ok?0:1;
    }
    if(command=="rebuild-projections"){
        // Retry only rebuildable views of already committed state. In
        // particular, never replay a scan or an Accepted recovery package.
        if(!store.writeAllProjections(&stateError)){error<<"Projection rebuild failed: "<<stateError<<"\n";return 1;}
        out<<"projections=current\n";return 0;
    }
    if(command=="validate"){
        archive::RecoveryImporter importer(config,store,logger);const auto result=importer.validate(QFileInfo(args[3]).absoluteFilePath());
        if(result.ok){out<<"VALID\npackage_id="<<result.packageId<<"\nitem_key="<<result.itemKey<<"\n";return 0;}
        error<<"INVALID\n"<<result.errors.join('\n')<<"\n";return 1;
    }
    if(command=="ingest-pending"){
        archive::RecoveryImporter importer(config,store,logger);QStringList failures,warnings;
        const auto accepted=importer.ingestPending(&failures,{},&warnings);
        out<<"accepted="<<accepted<<"\nwarnings="<<warnings.size()<<"\n";
        if(!failures.isEmpty())error<<failures.join('\n')<<"\n";
        if(!warnings.isEmpty())error<<warnings.join('\n')<<"\n";
        // Preserve failure=1. Exit 4 means acceptance committed but generated
        // reports need repair; callers must not resubmit those packages.
        return !failures.isEmpty()?1:!warnings.isEmpty()?4:0;
    }
    if(command=="scan"){
        auto sources=store.loadSources(&stateError);if(!stateError.isEmpty()){error<<stateError<<"\n";return 1;}
        archive::Source source;source.key=sourceKey;source.url=args[3];source.title=args.size()==5?args[4]:sourceKey;
        source.addedAt=QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
        for(const auto& previous:sources)if(previous.key==sourceKey){source=previous;source.url=args[3];if(args.size()==5)source.title=args[4];break;}
        archive::PlaylistDiscovery discovery(config,logger);const auto snapshot=discovery.discover(source);const auto result=store.reconcile(source,snapshot,&logger);
        out<<"source_key="<<sourceKey<<"\ncomplete="<<(snapshot.complete?"true":"false")<<"\nobserved="<<result.observed<<"\nactive="<<result.active<<"\nremoved="<<result.removed<<"\nunavailable="<<result.unavailable<<"\n";
        out<<"committed="<<(result.committed?"true":"false")<<"\nprojections="<<(!result.committed?"not_attempted":result.projectionsCurrent?"current":"dirty")<<"\n";
        if(!result.committed){error<<result.error<<"\n";return 1;}
        if(!result.projectionWarning.isEmpty())error<<result.projectionWarning<<"\n";
        if(!snapshot.complete)error<<snapshot.error<<"\n";
        // Partial discovery keeps its existing exit 3, even with dirty views.
        // A complete committed scan with dirty views has distinct exit 4.
        return !snapshot.complete?3:!result.projectionsCurrent?4:0;
    }
    if(command=="playlist-binding"){
        const auto playlist=store.loadPlaylistItems(sourceKey,&stateError);
        if(!stateError.isEmpty()){error<<stateError<<"\n";return 1;}
        const auto key="youtube:"+id;
        int activeOccurrences=0;
        QString entryKey;
        for(const auto& occurrence:playlist){
            if(occurrence.itemKey==key && occurrence.providerId==id && occurrence.membership=="active"){
                ++activeOccurrences;
                if(entryKey.isEmpty()) entryKey=occurrence.entryKey;
            }
        }
        out<<"source_key="<<sourceKey<<"\nitem_key="<<key<<"\nactive_occurrences="<<activeOccurrences<<"\n";
        if(!entryKey.isEmpty()) out<<"entry_key="<<entryKey<<"\n";
        out<<"member="<<(activeOccurrences>0?"true":"false")<<"\n";
        if(activeOccurrences<1){
            error<<"Requested video is not an active occurrence of the scanned playlist\n";
            return 1;
        }
        return 0;
    }
    const auto key="youtube:"+id;auto items=store.loadCanonicalItems(&stateError);
    if(!stateError.isEmpty()){error<<stateError<<"\n";return 1;}
    int index=-1;for(int i=0;i<items.size();++i)if(items[i].key==key){index=i;break;}
    if(command=="sync-item"){
        if(index<0){archive::CanonicalItem item;item.key=key;item.providerId=id;item.title=id;item.originalUrl="https://www.youtube.com/watch?v="+id;
            item.firstSeen=QDateTime::currentDateTime().toString(Qt::ISODateWithMs);item.lastSeen=item.firstSeen;
            items.append(item);index=items.size()-1;
            if(!store.saveCanonicalItems(items,&stateError)){error<<stateError<<"\n";return 1;}
        }
        archive::MediaExecutor executor(config,store,logger);
        if(!executor.syncItem(items[index],true,true,&stateError)){error<<"sync=FAIL item_key="<<key<<" error="<<stateError<<"\n";return 1;}
        items=store.loadCanonicalItems(&stateError);index=-1;for(int i=0;i<items.size();++i)if(items[i].key==key){index=i;break;}
    }
    if(index<0||!stateError.isEmpty()){error<<"Canonical item not found or unreadable\n";return 1;}
    const auto& item=items[index];archive::MediaVerifier verifier(config,logger);
    const auto video=verifier.verifyVideo(item.video.path),audio=verifier.verifyAudio(item.audio.path);
    const bool ok=item.video.state=="complete"&&item.audio.state=="complete"&&video.ok&&audio.ok;
    out<<(command=="sync-item"?"sync=":"verify=")<<(ok?"PASS":"FAIL")<<"\nitem_key="<<key<<"\nvideo_state="<<item.video.state<<"\nvideo_path="<<item.video.path<<"\naudio_state="<<item.audio.state<<"\naudio_path="<<item.audio.path<<"\n";
    if(!ok)error<<(video.errors+audio.errors).join('\n')<<"\n";return ok?0:1;
}
