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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>

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
    qputenv("ARCHIVE_TEST_CONFIG_ROOT",originalConfig);
    return failures?1:0;
}
