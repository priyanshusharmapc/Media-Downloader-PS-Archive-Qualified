#!/usr/bin/env python3
"""Regression policy for the post-225 forensic remediation batch."""
import argparse
import pathlib
import re
import sys


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def text(root, relative):
    return (root / relative).read_text(encoding="utf-8")


def section(source, start, end=None):
    begin = source.find(start)
    require(begin >= 0, f"missing section: {start}")
    if end is None:
        return source[begin:]
    finish = source.find(end, begin + len(start))
    require(finish >= 0, f"missing section end: {end}")
    return source[begin:finish]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=pathlib.Path,
                        default=pathlib.Path(__file__).resolve().parents[1])
    root = parser.parse_args().source_root.resolve()

    cmake = text(root, "CMakeLists.txt")
    deno = text(root, "src/engines/deno.cpp")
    qjs = text(root, "src/engines/quickjs_ng.cpp")
    engines_h = text(root, "src/engines.h")
    engines_cpp = text(root, "src/engines.cpp")
    settings_h = text(root, "src/settings.h")
    settings_cpp = text(root, "src/settings.cpp")
    utility_h = text(root, "src/utility.h")
    utility_cpp = text(root, "src/utility.cpp")
    logger_h = text(root, "src/logger.h")
    logger_cpp = text(root, "src/logger.cpp")
    network_h = text(root, "src/networkAccess.h")
    network_cpp = text(root, "src/networkAccess.cpp")
    playlist_h = text(root, "src/playlistdownloader.h")
    playlist_cpp = text(root, "src/playlistdownloader.cpp")
    batch_cpp = text(root, "src/batchdownloader.cpp")
    configure_h = text(root, "src/configure.h")
    configure_cpp = text(root, "src/configure.cpp")
    svt = text(root, "src/engines/svtplay-dl.cpp")
    ytdlp = text(root, "src/engines/yt-dlp.cpp")
    single = text(root, "src/utils/single_instance.hpp")
    qtimer = text(root, "src/utils/qtimer.hpp")
    library = text(root, "src/library.cpp")

    # 226: the only supported Qt5 floor has startDetached(), so the updater
    # never waits for a child while retaining its startup lock.
    require("find_package(Qt5 5.10 COMPONENTS Core QUIET)" in cmake,
            "226: Qt5 minimum is below detached-updater support")
    updater_start = section(utility_cpp, "static bool _start_updated", "bool utility::isOwnedUpdateCleanupPath")
    require("startDetached()" in updater_start and "waitForFinished" not in updater_start,
            "226: updater handoff can still synchronously wait")

    # 227/228: Deno removal identity and regenerated schema match production.
    require('platformIsWindows() ? "deno.exe" : "deno"' in deno,
            "227: Windows Deno removal does not target deno.exe")
    require('mainObj.insert( "Version","3" )' in deno,
            "228: regenerated Deno definition uses stale schema")
    require("runContainedProcess" in deno and "10000" in deno,
            "213/228: Deno version probing is not bounded")

    # 229: all Windows ARM64 QuickJS-ng paths intentionally use the same x64
    # emulation payload until upstream provides a native ARM64 binary.
    require('data.emplace_back( "aarch64","qjs-windows-x86_64.exe" )' in qjs,
            "229: Windows ARM64 command map is inconsistent")
    require("cpu.x86_64() || cpu.aarch64()" in qjs,
            "229: Windows ARM64 discovery/removal is inconsistent")

    # 230/231: menus are fresh snapshots and delayed actions own their metadata.
    require("static auto s = this->openWith()" not in settings_cpp,
            "230: Open With discovery is still process-lifetime cached")
    require("return { *this,this->openWith(),logger }" in settings_cpp,
            "230: Open With does not refresh discovery")
    require("QJsonObject m_obj" in settings_h and "settings::mediaPlayer::PlayerOpts m_playerOpts" in settings_h,
            "231: delayed Open With action still retains stack metadata by reference")

    # 232/238: logger state remains terminal/empty-safe.
    require("m_logger.registerDone()" in utility_h,
            "232: FailedToStart logs are not marked terminal")
    require("m_processOutputs.empty()" in logger_h and "m_entries && !m_entries->empty()" in logger_h,
            "238: active-log clearing can still leave unchecked empty dereferences")

    # 233/239/222/223: component update attempts are private until a locked,
    # trusted final promotion.
    require(".mdps-component-download-" in network_h,
            "239: component downloads still share a predictable path")
    require(".mdps-component-update.lock" in network_cpp,
            "239: component live promotion lacks cross-process serialization")
    require("QPointer< QWidget > guard" in network_cpp and
            "parent->extractArchiveOuput" in network_cpp,
            "233: extraction completion is not owner guarded")
    require("missing a valid trusted SHA-256 digest" in network_cpp,
            "222: component updater still accepts absent/malformed digest")
    require("updaterTreeIsSafe( opts.updateStagePath" in network_cpp,
            "223: extracted component tree is not validated before promotion")

    # 234/237: architecture and version-index contracts fail closed.
    commands = section(engines_cpp, "engines::engine::cmd engines::engine::getCommands",
                       "engines::engine::cmd::cmd")
    require("cpu.aarch32()" in commands and 'getCmd( cmd,"arm" )' in commands,
            "234: ARM32 does not have an explicit resolver path")
    require("Unknown or unsupported architectures fail closed" in commands,
            "234: unsupported CPUs can still fall through to amd64")
    version = section(engines_cpp, "QString engines::engine::versionString",
                      "QString engines::engine::setVersionString")
    require("m_line >= 0" in version and "m_position >= 0" in version,
            "237: negative version indices remain accepted")

    # 235/241: history read/modify/write is validated, atomic and locked across
    # both threads and supported processes.
    history = section(utility_cpp, "void utility::archiveData::addToHistory",
                      "QByteArray utility::archiveData::logHistoryData")
    require("QLockFile lock" in history and "QJsonParseError" in history and
            "QSaveFile out" in history,
            "235/241: history writer is not a locked validated atomic transaction")
    require("QLockFile lock( filePath + \".lock\" )" in utility_cpp,
            "241: history reads lack the interprocess lock")
    require("QLockFile lock( e + \".lock\" )" in logger_cpp,
            "241: history clear lacks the interprocess lock")

    # 236: plugin Name can never become an arbitrary filesystem authority.
    require("safePluginIdentity" in engines_cpp and
            "definitionFile == name + \".json\"" in engines_cpp,
            "236: custom plugin identity is not path constrained")
    require("updaterTreeIsSafe( folder.filePath()" in engines_cpp,
            "236: plugin payload deletion does not validate its tree")

    # 240: cancellation cleanup finishes while the cancelled job still owns the
    # pathname, rather than being detached past a retry.
    cleanup = section(utility_cpp, "void utility::deleteTmpFiles",
                      "bool utility::Qt6Version")
    require("utils::qthread::run" not in cleanup and "QFile::remove" in cleanup,
            "240: cancelled output cleanup is still detached")

    # 242: progress estimation refuses zero/non-finite denominators/results.
    require("std::isfinite( x )" in svt and "y <= 0.0" in svt and
            "std::isfinite( estimate )" in svt,
            "242: svtplay progress arithmetic remains non-finite capable")

    # 243: an empty option field survives menu metadata tokenization.
    require("Qt::KeepEmptyParts" in batch_cpp and "Qt::KeepEmptyParts" in playlist_cpp,
            "243: empty preset option text is still collapsed")

    # 244: migration, active yt-dlp work and Clear Archive use the same
    # interprocess lock namespace.
    require('migrationLock( current + ".lock" )' in engines_h,
            "244: legacy archive migration is not serialized")
    require("m_internalArchiveLocks" in playlist_h and
            "acquireInternalArchiveLock" in playlist_cpp and
            'QLockFile lock( path + ".lock" )' in playlist_cpp,
            "244: active download/Clear Archive ownership is not serialized")

    # 245/119: shared JSON stores keep a raw generation, reject stale writes and
    # never convert malformed persisted bytes into an editable empty model.
    require("m_baseline" in configure_h and "m_storeValid" in configure_h,
            "245/119: Configure stores do not track validity/generation")
    require(configure_cpp.count("QLockFile lock( m_path + \".lock\" )") >= 2 and
            configure_cpp.count("current != m_baseline") >= 2,
            "245: Configure stale snapshots can still overwrite concurrent edits")
    require("m_baseline" in playlist_h and
            "QLockFile lock( m_path + \".lock\" )" in playlist_cpp and
            "current != m_baseline" in playlist_cpp,
            "245: subscription stale snapshots can still overwrite concurrent edits")

    # 246: shutdown writers serialize the merge; startup atomically claims a
    # specific generation before the asynchronous consumer starts.
    autosave = section(utility_cpp, "void utility::saveDownloadList",
                       "utility::uiIndex::uiIndex")
    require('QLockFile autosaveLock( e + ".lock" )' in autosave,
            "246: autosave merge/replace is not interprocess serialized")
    require(".consume-" in batch_cpp and 'QLockFile lock( shared + ".lock" )' in batch_cpp,
            "246: startup restore does not claim an immutable generation")

    # 247/248: Windows uses routing state, and module-path retrieval grows
    # instead of accepting a truncated fixed buffer.
    require("GetBestRoute( 0,0,&route )" in utility_cpp,
            "247: Windows gateway still follows adapter enumeration order")
    app_path = section(utility_cpp, "QString utility::windowsApplicationDirPath",
                       "class adaptorInfo")
    require("std::vector< wchar_t > buffer" in app_path and
            "capacity <= 65536" in app_path and
            "std::array< wchar_t,4096 > buffer" not in app_path,
            "248: Windows executable path still accepts fixed-buffer truncation")

    # 061: sender waits for a receiver acknowledgement of a complete framed
    # payload; receiver validates the declared size before publishing.
    require('QByteArray::number( payload.size() ) + "\\n" + payload' in single,
            "061: secondary IPC is not length framed")
    require('acknowledgement.startsWith( "OK\\n" )' in single and
            's->write( "OK\\n",3 )' in single,
            "061: single-instance delivery has no receiver acknowledgement")
    require("bytes->size() > *expected" in single and "maxEventBytes" in single,
            "061: receiver does not reject incomplete/oversized framing")

    # 085: every delayed callback in the reopened scopes is bound to the actual
    # owner lifetime or a guarded QObject; async file reads require an owner.
    require("void run( QObject * context,int interval" in qtimer,
            "085: owner-aware delayed timer primitive missing")
    require("m_lifetimeContext" in network_h and
            "qtimer::run( m_args.lifetimeContext" in network_h,
            "085: delayed network retry retains an unguarded owner")
    require("static void readAll( QObject * context" in engines_h and
            "qthread::run( context" in engines_h,
            "085: async file reader is not owner guarded")
    require("qthread::run( this,meaw( this,safePath" in library,
            "085: Library scan delivery is not owner guarded")
    require("std::shared_ptr< QStringList > m_args" in settings_h,
            "085: Flatpak VLC probe still stores a raw owner")

    # 165: provider duration cannot terminate an EXTINF line.
    require("durationValue = m_obj.value( \"duration\" ).toString().toLongLong" in settings_cpp,
            "165: Flatpak EXTINF duration remains unsanitized")

    # 213: all known production version probes are bounded/cancellable and no
    # legacy indefinite updater wait remains.
    require("runContainedProcess" in engines_h and "10000" in engines_h and
            "std::atomic_bool" in engines_h,
            "213: normal engine version refresh is not bounded/cancellable")
    require("waitForFinished( -1 )" not in utility_cpp and
            "waitForFinished()" not in settings_cpp and
            "waitForFinished()" not in deno,
            "213: an unbounded production probe remains")

    # 214: app update work is private until update_new is replaced under the
    # same startup lock used by restart promotion.
    require(".mdps-app-update-" in network_cpp,
            "214: application update downloads still share an attempt path")
    require('.mdps-updater-startup.lock' in network_cpp and
            "removeFolder( md.finalPath )" in network_cpp,
            "214: update_new promotion is not serialized with startup")

    # 221: generic release selection requires exactly one predicate match.
    github_parser = section(utility_h, "QJsonObject parseJsonDataFromGitHub",
                            "class uiIndex")
    require("matches > 1" in github_parser or "++matches > 1" in github_parser,
            "221: first broad release-asset match can still win")
    require("matches != 1" in github_parser,
            "221: zero/ambiguous release assets do not fail closed")

    # 225: yt-dlp duration ingest stays qint64.
    require("static_cast< qint64 >( value )" in ytdlp and
            "static_cast< int >( dd.toDouble() )" not in ytdlp and
            "dd.toInt()" not in ytdlp,
            "225: yt-dlp duration still narrows to signed 32-bit")

    print("post-remediation 226-248 policy invariants: PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"post-remediation 226-248 policy invariants: FAIL: {exc}", file=sys.stderr)
        raise
