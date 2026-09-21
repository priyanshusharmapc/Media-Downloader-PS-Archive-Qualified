#!/usr/bin/env python3
"""Cross-cutting regression policy for the canonical audit2 integration."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",type=Path,default=Path(__file__).resolve().parents[1])
root=p.parse_args().source_root.resolve()
def read(path:str)->str:
    return (root/path).read_text(encoding="utf-8")

library=read("src/library.cpp")
entries=read("src/directoryEntries.cpp")
network=read("src/networkAccess.cpp")
quickjs=read("src/engines/quickjs.cpp")
svt=read("src/engines/svtplay-dl.cpp")
single=read("src/utils/single_instance.hpp")
workflow=read(".github/workflows/archive-qt6.yml")
cmake=read("CMakeLists.txt")
engine_tests=read("src/engines/tests.cpp")
library_h=read("src/library.h")

assert "directoryManager::removeEntryNative" in library
assert "directoryManager::renameEntryNative" in library
assert "directoryManager::removeDirectoryContentsNative" in library
assert "m_pendingActionNativeDirectory = m_currentNativePath" in library
assert "deleteAll( m_pendingActionNativeDirectory )" in library
assert "MDPS_LIBRARY_TEST_HOOKS" in library_h
assert "testPendingDirectoryMatches" in library_h
assert "testRemoveNativeEntry" in library_h
assert "testRemoveNativeDirectoryContents" in library_h
assert "--media-downloader-test-engine-library-native-mutations" in engine_tests
assert "library::testPendingDirectoryMatches" in engine_tests
assert "library::testRemoveNativeEntry" in engine_tests
assert "library::testRemoveNativeDirectoryContents" in engine_tests
assert "library-native-mutations" in cmake
assert "MDPS_LIBRARY_TEST_HOOKS=1" in cmake
for token in ("openat(", "O_NOFOLLOW", "fstatat(", "AT_SYMLINK_NOFOLLOW", "unlinkat(", "renameat("):
    assert token in entries
assert "if( !keepGoing.load() )return false" in entries

assert 'expected.startsWith( "sha256:",Qt::CaseInsensitive )' in network
assert "expected = expected.mid( 7 ).trimmed().toLower()" in network
assert "renameArchiveFolder( opts.metadata.fileName(),opts.updateStagePath )" in network
assert 'obj.insert( "DownloadUrl","" )' in quickjs and 'obj.insert( "AutoUpdate",false )' in quickjs
assert 'obj.insert( "DownloadUrl","" )' in svt and 'obj.insert( "AutoUpdate",false )' in svt

assert "m_lockFile.tryLock( 0 )" in single
assert "staleEndpointError" in single
assert "QLocalServer::removeServer" in single
assert "QFile::remove( m_info.socketPath )" not in single
start=single[single.index("void start()"):]
assert "m_lockFile.unlock()" not in start.split("QLocalServer m_localServer",1)[0]

assert "python3 -m compileall -q tests" in workflow
assert "python -m compileall -q tests" in workflow
assert "actions/upload-artifact@" not in workflow
assert "cache: false" in workflow
assert "artifact-gc:" in workflow and "actions: write" in workflow
assert "/actions/artifacts/" in workflow and "/actions/caches" in workflow
print("audit2 final integration policy: PASS")
