#include "../src/archive/archivesettings.h"
#include "../src/archive/archivetab.h"
#include "../src/archive/archivecore.h"

#include <QApplication>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QLockFile>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QMessageBox>
#include <QThread>

#include <functional>
#include <stdexcept>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
void require(bool ok,const QString& message)
{
    if(!ok)throw std::runtime_error(message.toStdString());
}

// Each case owns its settings and package directories, including when CTest
// runs several configurations concurrently on the same machine.
struct Fixture {
    QTemporaryDir temporary{QDir(qEnvironmentVariable("ARCHIVE_TEST_TMP",QDir::tempPath())).filePath("archive-gui-settings-XXXXXX")};
    QString app,downloads,root,config;
    Fixture()
    {
        require(temporary.isValid(),"Cannot create isolated fixture");
        app=temporary.filePath("package/app");
        downloads=temporary.filePath("downloads");
        root=temporary.filePath("archive");
        config=temporary.filePath("config");
        for(const auto& path:{app,downloads,root,config})require(QDir().mkpath(path),"Cannot create fixture directory");
        qputenv("ARCHIVE_TEST_CONFIG_ROOT",config.toUtf8());
    }
    QString resolve()const{return archive::ui::configuredRoot(app,downloads);}
    void writeSetting(const QString& key,const QVariant& value)
    {
        QSettings settings(archive::ui::settingsFile(),QSettings::IniFormat);
        settings.setValue(key,value);settings.sync();
        require(settings.status()==QSettings::NoError,"Cannot write fixture settings");
    }
};

void settingsStayOutsidePackage()
{
    Fixture f;
    require(archive::ui::persistRoot(f.root),"Cannot persist fixture root");
    require(QFileInfo::exists(archive::ui::settingsFile()),"Settings were not persisted");
    require(!QFileInfo::exists(QDir(f.app).filePath("local/settings/archive-mode.ini")),"Package seal was modified");
    require(f.resolve()==QDir::cleanPath(f.root),"Existing configured root changed");
}

void offlineRootNeverFallsBack()
{
    Fixture f;
    require(archive::ui::persistRoot(f.root),"Cannot persist fixture root");
    const auto offline=f.root+"-offline";
    require(QDir().rename(f.root,offline),"Cannot simulate offline volume");
    require(f.resolve()==f.root,"MDPS-AUDIT2-001: unavailable root silently selected a fallback");
    require(!QFileInfo::exists(f.root),"Resolution recreated an unavailable root");
    require(!QFileInfo::exists(QDir(f.downloads).filePath("State")),"Resolution mutated Downloads");
    require(QDir().rename(offline,f.root),"Cannot restore volume");
    require(f.resolve()==f.root,"Returning volume did not retain its configuration");
}

void unconfiguredRootIsNotInferred()
{
    Fixture f;
    const auto parent=QFileInfo(f.app).absolutePath();
    for(const auto& name:{"Video","Audio","State"})require(QDir().mkpath(QDir(parent).filePath(name)),"Cannot create generic directory");
    require(f.resolve().isEmpty(),"MDPS-AUDIT2-001: generic directories were inferred as an Archive Root");
    require(!QFileInfo::exists(QDir(parent).filePath("State/ArchiveMode")),"Resolution initialized generic State");
}

void legacyOfflineRootIsPreserved()
{
    Fixture f;
    const auto relative=QDir(f.app).relativeFilePath(f.root);
    f.writeSetting("ArchiveRootRelativeToApp",relative);
    require(QDir().rmdir(f.root),"Cannot remove legacy fixture root");
    require(f.resolve()==f.root,"MDPS-AUDIT2-001: unavailable explicitly configured legacy root was replaced");
}

void explicitEmptyRootDoesNotReviveLegacy()
{
    Fixture f;
    f.writeSetting("ArchiveRootRelativeToApp",QDir(f.app).relativeFilePath(f.root));
    f.writeSetting("ArchiveRoot",QString());
    require(f.resolve().isEmpty(),"MDPS-AUDIT2-001: empty explicit setting revived a different legacy root");
}

QByteArray contents(const QString& path)
{
    QFile file(path);require(file.open(QIODevice::ReadOnly),"Cannot read fixture "+path);
    const auto bytes=file.readAll();require(file.error()==QFileDevice::NoError,"Read failed");return bytes;
}

void put(const QString& path,const QByteArray& bytes)
{
    QFile file(path);require(file.open(QIODevice::WriteOnly),"Cannot write fixture "+path);
    require(file.write(bytes)==bytes.size(),"Short fixture write");require(file.flush(),"Fixture flush failed");
}

QString displayedRoot(QTabWidget& host)
{
    const auto* label=host.findChild<QLabel*>("archiveRootPath");
    require(label!=nullptr,"Archive root label missing");return QDir::fromNativeSeparators(label->text());
}

void checkReadiness(QTabWidget& host,bool ready)
{
    const auto* browse=host.findChild<QPushButton*>("archiveBrowseRoot");
    require(browse&&browse->isEnabled(),"Browse must remain usable for recovery");
    int mutations=0;
    for(auto* button:host.findChildren<QPushButton*>()){
        if(button->text()=="Add Playlist"||button->text()=="Sync All"){
            require(button->isEnabled()==ready,"Mutation action readiness is incorrect");++mutations;
        }
    }
    require(mutations==2,"Did not inspect both mutation actions");
}

void guiLaunchIsOptIn()
{
    Fixture f;
    QTabWidget host;ArchiveTab tab(host);tab.init_done();
    tab.tabEntered();tab.disableAll();tab.enableAll();
    checkReadiness(host,false);
    require(!QFileInfo::exists(archive::ui::settingsFile()),"Launching GUI persisted an inferred root");
    require(!QFileInfo::exists(QDir(f.root).filePath("State")),"Launching GUI initialized an unselected root");
    require(!QFileInfo::exists(QDir(f.downloads).filePath("State")),"Launching GUI initialized Downloads");
    // Programmatic/queued action delivery must also fail closed, independent
    // of widget enablement. No modal error or archive is created here.
    bool exercised=false;
    for(auto* action:host.findChildren<QAction*>())if(action->text()=="Process External Imports"){
        action->trigger();exercised=true;
    }
    require(exercised,"Import action was not tested");
    require(!QFileInfo::exists(QDir(f.downloads).filePath("State")),"Import action initialized Downloads");
}

void guiOfflineRootAndReconnect()
{
    Fixture f;QString active,error;
    require(archive::ui::selectRoot(f.root,active,&error),error);
    const auto oldItems=contents(archive::Paths(f.root).itemsFile());
    const auto offline=f.root+"-offline";require(QDir().rename(f.root,offline),"Cannot disconnect fixture root");
    QTabWidget host;ArchiveTab tab(host);tab.init_done();
    require(displayedRoot(host)==f.root,"Offline GUI replaced configured identity");
    checkReadiness(host,false);
    tab.tabEntered();tab.enableAll();
    for(auto* search:host.findChildren<QLineEdit*>())search->setText("refresh while offline");
    for(auto* action:host.findChildren<QAction*>())if(action->text()=="Process External Imports")action->trigger();
    require(!QFileInfo::exists(f.root),"GUI health/read/action path recreated an offline root");
    require(QDir().rename(offline,f.root),"Cannot reconnect fixture root");
    tab.tabEntered();checkReadiness(host,true);
    require(displayedRoot(host)==f.root&&f.resolve()==f.root,"Reconnect changed root identity");
    require(contents(archive::Paths(f.root).itemsFile())==oldItems,"Reconnect rewrote canonical records");
}

void acceptedSelectionCommitsAfterValidation()
{
    Fixture f;QString error;
    QTabWidget host;ArchiveTab tab(host);tab.init_done();
    require(tab.setRoot(f.root,&error),error);
    const auto next=f.temporary.filePath("next");require(QDir().mkpath(next),"Cannot create candidate");
    require(tab.setRoot(next,&error),error);
    require(f.resolve()==next&&displayedRoot(host)==next,"Accepted root did not become current and durable");
    require(QFileInfo::exists(archive::Paths(next).sourcesFile()),"Accepted root not initialized");
    require(tab.setRoot(next,&error),"Repeated valid selection failed: "+error);
    QTabWidget reopenedHost;ArchiveTab reopened(reopenedHost);reopened.init_done();
    require(displayedRoot(reopenedHost)==next,"Accepted root did not survive restart");
}

void rejectedSelectionPreservesWorkingRoot()
{
    Fixture f;QString error;
    QTabWidget host;ArchiveTab tab(host);require(tab.setRoot(f.root,&error),error);
    const auto oldSettings=contents(archive::ui::settingsFile());
    const auto oldSources=contents(archive::Paths(f.root).sourcesFile());
    const auto bad=f.temporary.filePath("corrupt-candidate");require(QDir().mkpath(bad),"Cannot create candidate");
    require(archive::ui::initializeRoot(bad,&error),error);
    const QByteArray corrupt="{not-json";put(archive::Paths(bad).sourcesFile(),corrupt);
    require(!tab.setRoot(bad,&error)&&!error.isEmpty(),"MDPS-AUDIT2-032: corrupt candidate accepted");
    require(f.resolve()==f.root&&displayedRoot(host)==f.root,"MDPS-AUDIT2-032: rejected candidate replaced working root");
    require(contents(archive::ui::settingsFile())==oldSettings,"Rejected selection rewrote settings");
    require(contents(archive::Paths(f.root).sourcesFile())==oldSources,"Rejected selection changed old archive");
    require(contents(archive::Paths(bad).sourcesFile())==corrupt,"Rejected candidate evidence was discarded");
    const auto absent=f.temporary.filePath("absent-candidate");
    require(!tab.setRoot(absent,&error)&&!QFileInfo::exists(absent),"Absent candidate was recreated/accepted");
    require(!tab.setRoot(QString(),&error)&&!tab.setRoot("relative-root",&error),"Empty/relative root was accepted");
    QTabWidget reopenedHost;ArchiveTab reopened(reopenedHost);reopened.init_done();
    require(displayedRoot(reopenedHost)==f.root,"Restart forgot prior accepted root after rejection");
    checkReadiness(host,true);
}

void linkedSelectionPreservesWorkingRoot()
{
    Fixture f;QString error,current;
    require(archive::ui::selectRoot(f.root,current,&error),error);
    const auto target=f.temporary.filePath("linked-target");require(QDir().mkpath(target),"Cannot create link target");
    const auto linked=f.temporary.filePath("linked-candidate");
#ifdef Q_OS_WIN
    const auto nativeLink=QDir::toNativeSeparators(linked),nativeTarget=QDir::toNativeSeparators(target);
    require(CreateSymbolicLinkW(reinterpret_cast<LPCWSTR>(nativeLink.utf16()),reinterpret_cast<LPCWSTR>(nativeTarget.utf16()),0x3)!=0,"Cannot create required Windows symlink fixture");
#else
    require(QFile::link(target,linked),"Cannot create required symlink fixture");
#endif
    require(!archive::ui::selectRoot(linked,current,&error)&&!error.isEmpty(),"Unsafe linked candidate accepted");
    require(current==f.root&&f.resolve()==f.root,"Linked rejection lost previous root");
    require(!QFileInfo::exists(QDir(target).filePath("State")),"Linked target was mutated");
#ifdef Q_OS_WIN
    require(RemoveDirectoryW(reinterpret_cast<LPCWSTR>(nativeLink.utf16()))!=0,"Cannot remove isolated directory symlink");
#else
    require(QFile::remove(linked),"Cannot remove isolated link fixture");
#endif
}

void failedSettingsCommitDoesNotSwitchOrRetry()
{
    Fixture f;QString error,current;
    require(archive::ui::selectRoot(f.root,current,&error),error);
    const auto oldSettings=contents(archive::ui::settingsFile());
    const auto next=f.temporary.filePath("next");require(QDir().mkpath(next),"Cannot create candidate");
    QLockFile lock(archive::ui::settingsFile()+".lock");require(lock.tryLock(0),"Cannot hold conflicting settings lock");
    require(!archive::ui::selectRoot(next,current,&error)&&!error.isEmpty(),"Conflicting settings writer was ignored");
    require(current==f.root&&f.resolve()==f.root,"Failed persistence changed root");
    lock.unlock();QCoreApplication::processEvents();
    require(contents(archive::ui::settingsFile())==oldSettings,"Rejected write retried after lock released");
    require(QFileInfo::exists(archive::Paths(next).sourcesFile()),"Initialized candidate evidence was deleted on config failure");
    require(archive::ui::selectRoot(next,current,&error),"Explicit retry failed: "+error);
}

void unusableSettingsDestinationDoesNotSwitch()
{
    Fixture f;QString error,current;
    require(archive::ui::selectRoot(f.root,current,&error),error);
    const auto configFile=archive::ui::settingsFile();const auto retained=configFile+".retained";
    require(QFile::rename(configFile,retained)&&QDir().mkpath(configFile),"Cannot block settings destination");
    const auto next=f.temporary.filePath("next");require(QDir().mkpath(next),"Cannot create candidate");
    require(!archive::ui::selectRoot(next,current,&error)&&current==f.root,"Unwritable configuration switched live root");
    require(QDir().rmdir(configFile)&&QFile::rename(retained,configFile),"Cannot restore settings destination");
    QCoreApplication::processEvents();
    require(f.resolve()==f.root,"Failed settings save retried after destination recovered");
}

void validLegacyAndUnrelatedSettingsSurvive()
{
    Fixture f;QString error,current;
    f.writeSetting("ArchiveRootRelativeToApp",QDir(f.app).relativeFilePath(f.root));
    f.writeSetting("Other/Preference",QStringList{"one","two"});
    require(f.resolve()==f.root,"Valid legacy root was not readable");
    require(archive::ui::selectRoot(f.root,current,&error),error);
    QSettings settings(archive::ui::settingsFile(),QSettings::IniFormat);settings.sync();
    require(!settings.contains("ArchiveRootRelativeToApp"),"Accepted selection did not migrate legacy setting");
    require(settings.value("Other/Preference").toStringList()==QStringList({"one","two"}),"Selection lost unrelated settings");
}

void corruptSettingsArePreserved()
{
    Fixture f;const QByteArray broken="[Broken\nArchiveRoot=/wrong\n";
    put(archive::ui::settingsFile(),broken);QString error,current=f.root;
    require(archive::ui::configuredRoot(f.app,f.downloads,&error).isEmpty()&&!error.isEmpty(),"Malformed INI was silently accepted");
    require(!archive::ui::selectRoot(f.root,current,&error),"Corrupt settings were overwritten");
    require(contents(archive::ui::settingsFile())==broken,"Corrupt settings evidence was discarded");
}


void archiveHealthRequiresFreshIntegrityEvidence()
{
    Fixture f;archive::Paths paths(f.root);archive::Store store(paths);QString error;
    require(store.initialize(&error),error);
    archive::Source source;source.key="PLHEALTH";source.title="Health fixture";
    source.url="https://www.youtube.com/playlist?list=PLHEALTH";
    archive::PlaylistItem item;item.itemKey="youtube:abc123DEF45";item.providerId="abc123DEF45";
    item.title="Health item";item.position=1;item.availability="public";
    archive::Snapshot snapshot;snapshot.sourceKey=source.key;snapshot.complete=true;snapshot.items={item};
    require(store.reconcile(source,snapshot).committed,"seed archive health fixture");

    const QString videoRel="Video/health [abc123DEF45].mp4";
    const QString audioRel="Audio/health [abc123DEF45].m4a";
    const auto videoAbs=paths.absoluteFromRelative(videoRel),audioAbs=paths.absoluteFromRelative(audioRel);
    put(videoAbs,"video-bytes");put(audioAbs,"audio-bytes");
    auto verifiedAfterFiles=QDateTime::currentDateTimeUtc().addSecs(2).toString(Qt::ISODateWithMs);
    archive::Representation video;video.state="complete";video.path=videoRel;video.origin="automatic_download";video.verifiedAt=verifiedAfterFiles;
    archive::Representation audio;audio.state="complete";audio.path=audioRel;audio.origin="automatic_download";audio.verifiedAt=verifiedAfterFiles;
    require(store.updateRepresentation(item.itemKey,"video",video,&error),error);
    require(store.updateRepresentation(item.itemKey,"audio",audio,&error),error);
    require(archive::ui::persistRoot(f.root),"select GUI health fixture root");

    QTabWidget host;ArchiveTab tab(host);tab.init_done();
    auto* table=host.findChild<QTableWidget*>();require(table&&table->rowCount()==1,"Archive health table missing fixture row");
    require(table->item(0,5)&&table->item(0,5)->text()=="Protected","Fresh verified media did not show Protected");

    require(QFile::remove(videoAbs),"Cannot delete verified video fixture");
    tab.tabEntered();
    require(table->item(0,5)->text()=="Missing","MDPS-AUDIT2-019: deleted completed media still shows Protected");
    table->setCurrentCell(0,0);QCoreApplication::processEvents();
    for(auto* text:host.findChildren<QTextEdit*>())
        require(!text->toPlainText().contains("Video present: Yes"),"MDPS-AUDIT2-019: details still claim deleted video is present");

    put(videoAbs,"video-bytes");
    verifiedAfterFiles=QDateTime::currentDateTimeUtc().addSecs(2).toString(Qt::ISODateWithMs);
    video.verifiedAt=verifiedAfterFiles;require(store.updateRepresentation(item.itemKey,"video",video,&error),error);
    tab.tabEntered();require(table->item(0,5)->text()=="Protected","Restored freshly verified media did not recover Protected state");

    QFile changed(videoAbs);require(changed.open(QIODevice::Append),"Cannot reopen video for corruption fixture");
    require(changed.write("corruption")>0&&changed.flush(),"Cannot mutate video fixture");
    require(changed.setFileTime(QDateTime::currentDateTimeUtc().addSecs(10),QFileDevice::FileModificationTime),"Cannot advance corruption timestamp");
    changed.close();
    tab.tabEntered();
    require(table->item(0,5)->text()=="Verification Stale","MDPS-AUDIT2-019: modified completed media still shows Protected");
    table->setCurrentCell(0,0);QCoreApplication::processEvents();bool sawStale=false;
    for(auto* text:host.findChildren<QTextEdit*>())if(text->toPlainText().contains("Video integrity: Verification stale"))sawStale=true;
    require(sawStale,"Details do not distinguish persisted completion from stale integrity evidence");
}

void acceptedRecoveryWithReportWarning()
{
    Fixture f;
    archive::Paths paths(f.root);archive::Store store(paths);QString error;
    require(store.initialize(&error),error);
    archive::Source source;source.key="PLAUDIT";source.title="Projection outcome";
    source.url="https://www.youtube.com/playlist?list=PLAUDIT";
    archive::PlaylistItem item;item.itemKey="youtube:abc123DEF45";item.providerId="abc123DEF45";
    item.title="Historical title";item.position=1;item.availability="public";
    archive::Snapshot snapshot;snapshot.sourceKey=source.key;snapshot.complete=true;snapshot.items={item};
    require(store.reconcile(source,snapshot).committed,"seed committed source");
    require(archive::ui::persistRoot(f.root),"select GUI fixture root");
    const auto package=QDir(paths.importsPending()).filePath("outcome-gui");
    require(QDir().mkpath(package),"create pending GUI fixture");
    const QJsonObject manifest{{"schema_version",1},{"package_id","outcome-gui"},
        {"target",QJsonObject{{"item_key",item.itemKey},{"youtube_id",item.providerId}}},
        {"metadata",QJsonObject{{"canonical_title","Recovered metadata"}}},
        {"provenance",QJsonObject{{"method","old_local_backup"},{"confidence","high"},
            {"source_url","https://www.youtube.com/watch?v=abc123DEF45"}}}};
    put(QDir(package).filePath("manifest.json"),QJsonDocument(manifest).toJson());
    // Block only a derived report after the authoritative fixture is healthy.
    // The UI must still acknowledge acceptance when that later rebuild fails.
    const auto catalog=QDir(paths.sourceDir(source.key)).filePath("catalog.csv");
    require(QFile::remove(catalog)&&QDir().mkdir(catalog),"block report destination");
    QTabWidget host;ArchiveTab tab(host);tab.init_done();
    QAction* process=nullptr;
    for(auto action:host.findChildren<QAction*>())
        if(action->text()=="Process External Imports")process=action;
    require(process&&process->isEnabled(),"Pending import action unavailable");
    QTimer dismiss;
    QObject::connect(&dismiss,&QTimer::timeout,[]{
        for(auto widget:QApplication::topLevelWidgets())
            if(auto box=qobject_cast<QMessageBox*>(widget))box->accept();
    });
    dismiss.start(10);process->trigger();
    QElapsedTimer elapsed;elapsed.start();bool finished=false,warned=false,accepted=false;
    while(elapsed.elapsed()<10000&&!finished){
        QCoreApplication::processEvents();
        for(auto label:host.findChildren<QLabel*>()){
            if(label->text().startsWith("COMPLETED"))finished=true;
            if(label->text()=="COMPLETED WITH WARNINGS")warned=true;
            if(label->text().contains("Accepted imports 1"))accepted=true;
        }
        if(!finished)QThread::msleep(1);
    }
    require(finished,"GUI worker failed to complete");
    require(!QFileInfo::exists(package)&&QFileInfo(QDir(paths.importsAccepted()).filePath("outcome-gui")).isDir(),
        "recovery not actually accepted");
    require(warned&&accepted,"GUI concealed durable acceptance behind a report failure");
}

}

int main(int argc,char** argv)
{
    QApplication app(argc,argv);
    app.setOrganizationName("MediaDownloaderTest");
    app.setApplicationName("ArchiveGuiSettingsTest");
    QStandardPaths::setTestModeEnabled(true);
    const auto originalConfig=qgetenv("ARCHIVE_TEST_CONFIG_ROOT");
    int failures=0;
    const auto run=[&](const char* name,const std::function<void()>& test){
        try{test();QTextStream(stdout)<<name<<": PASS\n";}
        catch(const std::exception& e){++failures;QTextStream(stderr)<<name<<": FAIL: "<<e.what()<<"\n";}
    };
    run("settings-stay-outside-package",settingsStayOutsidePackage);
    run("offline-root-never-falls-back",offlineRootNeverFallsBack);
    run("unconfigured-root-not-inferred",unconfiguredRootIsNotInferred);
    run("legacy-offline-root-preserved",legacyOfflineRootIsPreserved);
    run("empty-explicit-root-no-legacy",explicitEmptyRootDoesNotReviveLegacy);
    run("gui-launch-opt-in",guiLaunchIsOptIn);
    run("gui-offline-root-reconnect",guiOfflineRootAndReconnect);
    run("accepted-selection-commit",acceptedSelectionCommitsAfterValidation);
    run("rejected-selection-retains-root",rejectedSelectionPreservesWorkingRoot);
    run("linked-selection-retains-root",linkedSelectionPreservesWorkingRoot);
    run("settings-conflict-no-retry",failedSettingsCommitDoesNotSwitchOrRetry);
    run("unusable-settings-no-switch",unusableSettingsDestinationDoesNotSwitch);
    run("legacy-and-unrelated-settings",validLegacyAndUnrelatedSettingsSurvive);
    run("corrupt-settings-preserved",corruptSettingsArePreserved);
    run("archive-health-fresh-integrity",archiveHealthRequiresFreshIntegrityEvidence);
    run("accepted-recovery-report-warning",acceptedRecoveryWithReportWarning);
    qputenv("ARCHIVE_TEST_CONFIG_ROOT",originalConfig);
    return failures?1:0;
}
