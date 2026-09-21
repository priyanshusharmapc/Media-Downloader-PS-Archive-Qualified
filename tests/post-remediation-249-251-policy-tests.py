#!/usr/bin/env python3
"""Regression policy for the final post-225 remediation scopes."""
import argparse
import pathlib
import sys


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def text(root, relative):
    return (root / relative).read_text(encoding="utf-8")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=pathlib.Path,
                        default=pathlib.Path(__file__).resolve().parents[1])
    root = parser.parse_args().source_root.resolve()

    themes = text(root, "src/themes.cpp")
    utility_h = text(root, "src/utility.h")
    utility_cpp = text(root, "src/utility.cpp")
    network = text(root, "src/networkAccess.cpp")
    library_h = text(root, "src/library.h")
    library = text(root, "src/library.cpp")
    entries_h = text(root, "src/directoryEntries.h")
    entries = text(root, "src/directoryEntries.cpp")
    settings_h = text(root, "src/settings.h")
    settings = text(root, "src/settings.cpp")
    ytdlp = text(root, "src/engines/yt-dlp.cpp")

    # 249: generated built-in themes are validated and replaced atomically.
    require("validBuiltInThemeFile" in themes and
            "QSaveFile file( path )" in themes and
            "file.setDirectWriteFallback( false )" in themes and
            "ensureBuiltInTheme( defaultDarkThemePath" in themes and
            "ensureBuiltInTheme( defaultPureDarkThemePath" in themes,
            "249: built-in theme bootstrap is not atomic/self-healing")
    require('if( !QFile::exists( defaultDarkThemePath ) )' not in themes,
            "249: existence-only malformed theme authority remains")

    # 041: executable permission repair is observable and activation checks it.
    require("bool setPermissions( QFile& )" in utility_h and
            "bool setPermissions( const QString& )" in utility_h,
            "041: permission repair still cannot report failure")
    require("return info.isExecutable()" in utility_cpp and
            "if( !utility::setPermissions( stagedExecutable ) )" in network and
            "if( !utility::setPermissions( finalExecutable ) )" in network and
            "if( !utility::setPermissions( opts.file.src() ) )" in network,
            "041: update activation does not fail closed on chmod failure")

    # 250: every archive extractor uses the bounded process-tree helper.
    require("runBoundedExtractor" in network and
            "archive::detail::runContainedProcess" in network and
            "extractionTimeoutMs" in network,
            "250: updater extractors lack a finite contained lifecycle")
    require("utils::qprocess::run( exe,args" not in network,
            "250: an unbounded direct extractor launch remains")

    # 137: queued Library row chains own immutable snapshots and generations.
    require("std::vector< snapshot > m_entries" in entries_h and
            "quint64 m_generation" in entries_h and
            "Iter( quint64 generation" in entries_h,
            "137: queued iterator still aliases mutable directory storage")
    require("m_populationGeneration" in library_h and
            "s.generation() != m_populationGeneration" in library and
            "const auto generation = ++m_populationGeneration" in library,
            "137: stale queued population events are not generation-bound")

    # 172: nightly Windows ARM64 uses the native release asset identity.
    require('data.emplace_back( "aarch64","yt-dlp_arm64-nightly.exe" )' in ytdlp,
            "172: yt-dlp nightly Windows ARM64 still falls back to amd64")

    # 251: raw POSIX names remain authoritative through listing and actions.
    require("QByteArray m_nativeName" in entries_h and
            "const QByteArray& nativeName() const" in entries_h and
            "readAllNative" in entries_h,
            "251: directory model does not retain native POSIX identity")
    require("QByteArray nativeName( name" in entries and
            "displayName.toUtf8() != nativeName" in entries and
            "entries.addFile( m.st_mtime,displayName,nativeName )" in entries and
            "entries.addFolder( m.st_mtime,displayName,nativeName )" in entries,
            "251: readdir bytes are still collapsed into QString identity")
    require("item.setData( Qt::UserRole,s.nativeName() )" in library and
            "nativePathAt" in library and
            "renameNativeEntry" in library and
            "removeDirectoryNative" in library and
            "readAllNative" in library,
            "251: Library actions are not routed through retained native identity")
    require("void openUrl( const QByteArray& nativePath )" in settings_h and
            "QUrl::fromEncoded( encoded,QUrl::StrictMode )" in settings,
            "251: exact native POSIX file open path is missing")

    print("post-remediation 249-251 final policy invariants: PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"post-remediation 249-251 final policy invariants: FAIL: {exc}",
              file=sys.stderr)
        raise
