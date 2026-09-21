#!/usr/bin/env python3
# Full remediation checkpoint: source fixes through the final 085/220 lifetime pass.
import argparse
import pathlib
import re
import sys


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def text(root, relative):
    return (root / relative).read_text(encoding="utf-8")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=pathlib.Path, default=pathlib.Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    root = args.source_root.resolve()

    cmake = text(root, "CMakeLists.txt")
    settings_h = text(root, "src/settings.h")
    settings_cpp = text(root, "src/settings.cpp")
    engines_h = text(root, "src/engines.h")
    engines_cpp = text(root, "src/engines.cpp")
    utility_h = text(root, "src/utility.h")
    utility_cpp = text(root, "src/utility.cpp")
    network_hpp = text(root, "src/utils/network_access_manager.hpp")
    network_cpp = text(root, "src/networkAccess.cpp")
    proxy_cpp = text(root, "src/proxy.cpp")
    main_cpp = text(root, "src/main.cpp")
    version_cpp = text(root, "src/versionInfo.cpp")
    archive_safety = text(root, "src/archive/archivesafety.h")
    archive_core = text(root, "src/archive/archivecore.cpp")
    archive_cli = text(root, "src/archive/archivecli.cpp")
    archive_tab = text(root, "src/archive/archivetab.cpp")
    archive_settings = text(root, "src/archive/archivesettings.cpp")
    playlist_cpp = text(root, "src/playlistdownloader.cpp")
    table_h = text(root, "src/tableWidget.h")
    table_cpp = text(root, "src/tableWidget.cpp")
    batch_h = text(root, "src/batchdownloader.h")
    batch_cpp = text(root, "src/batchdownloader.cpp")
    tab_h = text(root, "src/tabmanager.h")
    tab_cpp = text(root, "src/tabmanager.cpp")
    safari = text(root, "src/engines/safaribooks.cpp")
    ytdlp = text(root, "src/engines/yt-dlp.cpp")
    getsauce = text(root, "src/engines/getsauce.cpp")
    json_size = text(root, "src/engines/json_media_size.hpp")
    package_test = text(root, "tests/archive-gui-package-immutability.ps1")
    linux_build = text(root, "build_linux.sh")
    arch_build = text(root, "build_arch.sh")

    # 204: declaration/definition must have the same parameter order.
    signature = r"addToHistory\s*\(\s*QSettings&\s+\w+\s*,\s*QStringList&\s+\w+\s*,\s*const QString&\s+\w+"
    require(re.search(signature, settings_h), "204: header addToHistory signature is not canonical")
    require(re.search(signature, settings_cpp), "204: implementation addToHistory signature is not canonical")

    # 205/207: FindPython3 needs CMake 3.12, and policy tests are registered once.
    require("cmake_minimum_required(VERSION 3.12.0)" in cmake, "205: CMake minimum must support FindPython3")
    require(cmake.count('file(GLOB MDPS_POLICY_TESTS') == 1, "207: generic policy test discovery must be registered exactly once")
    require(cmake.count("MDPS_POLICY_NAME") >= 1, "207: policy tests must remain registered")

    # 206: explicit exports flow through an atomic writer and report failure.
    require("QSaveFile file( m_filePath )" in engines_cpp and "file.commit()" in engines_cpp, "206: shared writer must be atomic")
    require("Failed to save the list. The previous file was preserved." in utility_cpp, "206: list export must surface persistence failure")

    # 208/210/224: external-player playlists are exclusively created, atomically
    # written, and leased instead of being deleted during active handoff.
    require("QTemporaryFile file" in settings_cpp and 'handoff-XXXXXX.m3u8' in settings_cpp, "208: M3U names need exclusive creation")
    require("QSaveFile out( m )" in settings_cpp and "out.write( payload ) != payload.size()" in settings_cpp, "224: M3U write must be checked")
    require("leaseSeconds = 24 * 60 * 60" in settings_cpp, "210: M3U cleanup needs a lease")
    require("settings::~settings()" in settings_cpp and "this->clearFlatPakTemps()" not in settings_cpp.split("settings::~settings()",1)[1].split("}",1)[0],
            "210: destructor must not delete active handoffs")

    # 209: body bytes are delivered by data readiness and drained before terminal callback.
    require("&QIODevice::readyRead" in network_hpp, "209: network body delivery must use readyRead")
    require("bytesAvailable() > 0" in network_hpp and "m_readyReadConn" in network_hpp, "209: final unread tail must be handled")
    require("h.progress( r,t )" not in network_hpp, "209: downloadProgress must not call a removed/body-consuming progress path")

    # 211: production executable ignores qualification-only Archive hooks.
    require("ARCHIVE_GUI_TEST_HOOK" not in main_cpp, "211: production main still contains test mutation hook")
    require("#ifdef MDPS_ARCHIVE_TEST_HOOKS" in archive_settings, "211: config override is not compile-time test isolated")
    require("MDPS_ARCHIVE_TEST_HOOKS=1" in cmake, "211: test target must explicitly opt into hooks")
    require("Production executable honored a qualification-only Archive settings hook" in package_test,
            "211: packaged production isolation regression missing")

    # 004: desktop identities match the qualified fork.
    require('io.github.priyanshusharmapc.MediaDownloaderPSArchive' in main_cpp, "004: Flatpak desktop identity regressed")
    require('media-downloader-ps-archive' in main_cpp, "004: native desktop identity regressed")

    # 212: build scripts operate on their own checkout.
    for name, script in (("linux", linux_build), ("arch", arch_build)):
        require('SCRIPT_DIR=' in script and 'cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"' in script, f"212: {name} build is not checkout-bound")
        require("~/media-downloader" not in script, f"212: {name} build still contains hard-coded checkout")

    # 213: both engine and Flatpak player probes are bounded and lifetime guarded.
    require("runContainedProcess" in version_cpp and "10000" in version_cpp and "qthread::run( &m_ctx.mainWidget()" in version_cpp,
            "213: engine version probe is not bounded/guarded")
    require("runContainedProcess" in settings_cpp and "5000" in settings_cpp and "qthread::run( context" in settings_cpp,
            "213: Flatpak VLC probe is not bounded/guarded")
    updater_probe = utility_cpp[utility_cpp.find("static util::version _get_process_version"):utility_cpp.find("static bool _start_updated")]
    require("runContainedProcess" in updater_probe and "waitForFinished()" not in updater_probe,
            "213: staged updater version probe is still unbounded")

    # 214/221/222/223: application updater identity and filesystem trust boundary.
    require(".mdps-updater-startup.lock" in utility_cpp, "214: updater startup transaction is not serialized")
    require('MediaDownloaderQt6.zip' in network_cpp and 'MediaDownloaderQt5.zip' in network_cpp and "matches.size() != 1" in network_cpp,
            "221: updater asset identity is not exact and unique")
    require("release asset has no required SHA-256 digest" in network_cpp, "222: missing remote digest is still accepted")
    require("updaterTreeIsSafe" in utility_cpp and "FILE_ATTRIBUTE_REPARSE_POINT" in utility_cpp and "nNumberOfLinks != 1" in utility_cpp,
            "223: updater tree does not reject reparse/hard-link escapes")
    require("updaterTreeIsSafe( extractedPath" in network_cpp, "223: extracted tree is not validated before promotion")

    # 215/216/217: proxy route selection is validated, asynchronous and preference preserving.
    require("bestMetric" in proxy_cpp and "( flags & 0x3u ) != 0x3u" in proxy_cpp and "fields.size() < 7" in proxy_cpp,
            "215: Linux default route selection is not metric/flags validated")
    require("qthread::run( &ctx.mainWidget()" in proxy_cpp and "_proxyGeneration" in proxy_cpp,
            "216: Windows proxy discovery is not guarded/asynchronous")
    require("proxy.type() == QNetworkProxy::NoProxy" in proxy_cpp and "proxy.type() == QNetworkProxy::DefaultProxy" in proxy_cpp,
            "217: system proxy preference order does not preserve direct results")

    # 218/219: stale Archive locks are recoverable and every finalization failure
    # transitions away from running.
    require("removeStaleLockFile()" in archive_core and "getLockInfo" in archive_core, "218: stale lock recovery missing")
    require("Media binding finalization failed" in archive_core and "completion state could not be committed" in archive_core,
            "219: media finalization can still return with durable running state")

    # 220: background clipboard has explicit latest-event semantics and owner guard.
    require("m_clipboardGeneration" in tab_h and "m_generation != m_parent->m_clipboardGeneration" in tab_cpp,
            "220: clipboard callbacks are not generation ordered")
    require("qthread::run( &m_ctx.mainWidget()" in tab_cpp, "220/085: clipboard foreground callback is not lifetime guarded")

    # 021: persisted verification fingerprints are fully schema validated.
    require('"verified_sha256","verification_profile"' in archive_safety and "qint64ExclusiveUpper" in archive_safety,
            "021: representation fingerprint schema validation incomplete")
    require("n<qint64ExclusiveUpper" in archive_core, "021: verified_size cast boundary remains unsafe")

    # 026/038: external metadata writes stay inside a fresh app-owned stage and
    # canonical metadata_path is the exact published directory, never a scan.
    require("QTemporaryDir metadataStaging" in archive_core and "validateOwnedStagingTree" in archive_core,
            "038: metadata external writer is not staged safely")
    require('Metadata/youtube-' in archive_core and "metadataPath=metadataPathCandidate" in archive_core,
            "026: metadata path is not bound to this exact run")
    require('entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name)' not in archive_core.split("bool MediaExecutor::downloadVideo",1)[1].split("bool MediaExecutor::downloadAudio",1)[0],
            "026: video completion still scans metadata alphabetically")

    # 048: empty tokenization must never reach first().
    require("tokens.isEmpty()" in playlist_cpp and "tokens.first().trimmed().isEmpty()" in playlist_cpp,
            "048: playlist list input still has empty-token access")

    # 074: size paths use validated 64-bit conversion.
    require("qint64* out" in json_size and "largestExactJsonInteger" in json_size, "074: shared JSON size parser is not qint64-safe")
    require("nonNegativeByteCount" in ytdlp and '.toInt( -1 )' not in ytdlp[ytdlp.find("QString fileSizeRaw"):ytdlp.find("void append", ytdlp.find("QString fileSizeRaw"))],
            "074: yt-dlp size path still narrows to int")
    require("nonNegativeByteCount" in getsauce, "074: GetSauce size path still narrows to int")

    # 075: metadata and thumbnail completions bind to immutable row identity.
    require("stableIdentity" in table_h and "QUuid::createUuid" in table_cpp and "rowWithIdentity" in table_h,
            "075: stable row identity primitive missing")
    require("rowWithIdentity( identity )" in batch_cpp and "networkCtx( media,index,m_table.entryAt( index ).stableIdentity )" in batch_cpp,
            "075: async metadata/thumbnail callbacks still resolve by URL/index")

    # 077: SafariBooks command diagnostics must never emit --cred payload.
    safari_command = safari[safari.find("QString safaribooks::commandString"):safari.find("void safaribooks::sendCredentials")]
    require("<REDACTED>" in safari_command and 'm += " \\"" + args[ i ] + "\\"" ;' not in safari_command.split('args[ i - 1 ] == "--cred"',1)[1].split("}else",1)[0],
            "077: SafariBooks command renderer still exposes credentials")

    # 085: async file reads require an owner QObject and do no owner work in bg.
    require("static void readAll( QObject * context" in engines_h and "qthread::run( context" in engines_h,
            "085: async file reader still permits context-free owner callback")
    require("engines::file::readAll( this,e,m_ctx.logger()" in batch_cpp,
            "085: Batch restore/import does not bind async read to controller lifetime")

    # 093/112/225: durations use elapsed arithmetic and checked qint64 inputs.
    duration_body = engines_cpp[engines_cpp.find("timer::duration"):engines_cpp.find("timer::toSeconds")]
    require("QTime" not in duration_body and 'QString( "%1:%2:%3" )' in duration_body, "093: elapsed duration still uses time-of-day")
    to_seconds = engines_cpp[engines_cpp.find("timer::toSeconds"):engines_cpp.find("timer::elapsedTime")]
    require("toLongLong" in to_seconds and "numeric_limits< int >::max()" in to_seconds, "112: duration parser arithmetic is not checked")
    require("qint64 m_intDuration" in utility_h and "largestSafeSeconds" in utility_cpp and "1000LL" in utility_cpp,
            "225: media duration storage/multiplication still narrows to int")
    require("numeric_limits< qint64 >::max() / 1000LL" in batch_cpp,
            "225: imported yt-dlp list duration still has unchecked int multiplication")

    # 183: unregister preserves explicit historical ownership evidence.
    require("retired.json" in archive_tab and "user_unregistered" in archive_tab,
            "183: playlist removal does not persist retirement marker")
    require("retiredPath" in archive_core and "Invalid retired playlist marker" in archive_core,
            "183: graph validation cannot distinguish intentional retirement")

    # 196: CLI first admission requires a complete provider snapshot.
    require("alreadyRegistered" in archive_cli and "if(!alreadyRegistered&&!snapshot.complete)" in archive_cli,
            "196: CLI can still persist incomplete first playlist admission")

    print("post-remediation 204-225 policy invariants: PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"post-remediation 204-225 policy invariants: FAIL: {exc}", file=sys.stderr)
        raise
