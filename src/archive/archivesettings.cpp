#include "archivesettings.h"
#include "archivecore.h"
#include "archivesafety.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLockFile>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>

namespace archive { namespace ui {
namespace {
QString message(const char* text)
{
    return QCoreApplication::translate("ArchiveSettings",text);
}

QString settingsPath()
{
    const auto testRoot=qEnvironmentVariable("ARCHIVE_TEST_CONFIG_ROOT");
    if(!testRoot.isEmpty())return QDir(testRoot).filePath("archive-mode.ini");
    auto directory=QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if(directory.isEmpty())directory=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if(directory.isEmpty())directory=QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath("MediaDownloader");
    // Reads must not create config directories or imply operator opt-in.
    return QDir(directory).filePath("archive-mode.ini");
}

// QSettings shares file-backed caches and can flush pending changes from its
// destructor. Work on a private snapshot instead: a failed live-file commit
// must not leave a rejected root queued for a later implicit retry.
bool stageSettings(const QString& path,QTemporaryFile& staged,QString* error)
{
    if(!detail::noLinks(path))return detail::reject(error,message("Linked Archive settings path refused: %1").arg(path));
    QByteArray bytes;
    if(QFileInfo::exists(path)&&!detail::readBytes(path,&bytes,error))return false;
    if(!staged.open()||staged.write(bytes)!=bytes.size()||!staged.flush())
        return detail::reject(error,message("Cannot stage Archive settings: %1").arg(staged.errorString()));
    staged.close();
    return true;
}

bool validSettings(QSettings& settings,QString* error)
{
    // allKeys forces lazy INI parsing, including unrelated sections. Corrupt
    // settings are evidence to preserve, not permission to replace the file.
    settings.allKeys();
    if(settings.status()!=QSettings::NoError)
        return detail::reject(error,message("Cannot read Archive settings; preserve the file and repair its format or permissions."));
    return true;
}

bool absoluteRoot(const QString& root,QString* error)
{
    if(root.trimmed().isEmpty())return detail::reject(error,message("No Archive Root selected. Use Browse to select an existing directory."));
    if(!QDir::isAbsolutePath(root)||root.contains(QChar(0)))
        return detail::reject(error,message("Archive Root must be an absolute directory path: %1").arg(root));
    return true;
}
}

QString settingsFile(){return settingsPath();}

QString configuredRoot(const QString& applicationDir,const QString& downloadFolder,QString* error)
{
    Q_UNUSED(downloadFolder)
    if(error)error->clear();
    const auto path=settingsPath();
    if(!QFileInfo::exists(path)&&detail::noLinks(path))return {};
    QTemporaryFile staged(QDir::tempPath()+"/mdps-settings-read-XXXXXX.ini");
    if(!stageSettings(path,staged,error))return {};
    QSettings settings(staged.fileName(),QSettings::IniFormat);
    settings.setFallbacksEnabled(false);
    if(!validSettings(settings,error))return {};

    // Presence, not availability, determines precedence. An empty/malformed
    // explicit setting must not revive a different legacy or guessed archive.
    if(settings.contains("ArchiveRoot")){
        const auto stored=settings.value("ArchiveRoot").toString();
        return stored.isEmpty()?QString():QDir::cleanPath(stored);
    }
    const auto relative=settings.value("ArchiveRootRelativeToApp").toString();
    if(relative.isEmpty())return {};
    // Preserve pre-release configuration without mutating it on launch. The
    // next accepted selection migrates it to an absolute setting atomically.
    return QDir::cleanPath(QDir(applicationDir).filePath(relative));
}

bool rootAvailable(const QString& root,QString* error)
{
    if(error)error->clear();
    if(!absoluteRoot(root,error))return false;
    if(!detail::noLinks(root))return detail::reject(error,message("Linked Archive Root refused: %1").arg(root));
    if(!QFileInfo(root).isDir())
        return detail::reject(error,message("Configured Archive Root is unavailable: %1. Reconnect it or explicitly select another root; no fallback was opened.").arg(root));
    return true;
}

bool initializeRoot(const QString& root,QString* error)
{
    if(!rootAvailable(root,error))return false;
    Store store{Paths(root)};
    return store.initialize(error);
}

bool persistRoot(const QString& root,QString* error)
{
    if(error)error->clear();
    if(!absoluteRoot(root,error))return false;
    const auto path=settingsPath();
    if(!detail::noLinks(path)||!detail::noLinks(path+".lock"))
        return detail::reject(error,message("Linked Archive settings path refused: %1").arg(path));
    if(!QDir().mkpath(QFileInfo(path).absolutePath()))
        return detail::reject(error,message("Cannot create Archive settings directory: %1").arg(QFileInfo(path).absolutePath()));

    // Use the same lock path as legacy QSettings writers. Do not hold the UI
    // indefinitely for another process or overwrite a concurrent selection.
    QLockFile lock(path+".lock");
    if(!lock.tryLock(0))return detail::reject(error,message("Archive settings are busy or cannot be locked. The selected root was not changed."));
    QTemporaryFile staged(QDir::tempPath()+"/mdps-settings-write-XXXXXX.ini");
    if(!stageSettings(path,staged,error))return false;
    {
        QSettings settings(staged.fileName(),QSettings::IniFormat);
        settings.setFallbacksEnabled(false);
        if(!validSettings(settings,error))return false;
        settings.setValue("ArchiveRoot",QDir::cleanPath(root));
        settings.remove("ArchiveRootRelativeToApp");
        settings.sync();
        if(settings.status()!=QSettings::NoError)
            return detail::reject(error,message("Cannot serialize Archive settings. The selected root was not changed."));
    }
    QFile serialized(staged.fileName());
    if(!serialized.open(QIODevice::ReadOnly))return detail::reject(error,serialized.errorString());
    const auto bytes=serialized.readAll();
    if(serialized.error()!=QFileDevice::NoError)return detail::reject(error,serialized.errorString());
    if(!detail::noLinks(path))return detail::reject(error,message("Archive settings path changed during selection."));
    QSaveFile committed(path);
    committed.setDirectWriteFallback(false);
    if(!committed.open(QIODevice::WriteOnly)||committed.write(bytes)!=bytes.size()||!committed.commit())
        return detail::reject(error,message("Cannot commit Archive settings: %1. The selected root was not changed.").arg(committed.errorString()));
    return true;
}

bool selectRoot(const QString& candidate,QString& currentRoot,QString* error)
{
    // Do not clean an empty candidate into '.', or resolve relative input
    // against the process working directory before checking operator intent.
    if(!absoluteRoot(candidate,error))return false;
    const auto accepted=QDir::cleanPath(candidate);
    if(!initializeRoot(accepted,error)||!persistRoot(accepted,error))return false;
    currentRoot=accepted;
    return true;
}
} }
