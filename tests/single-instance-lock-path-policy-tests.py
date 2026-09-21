"""Windows single-instance lock stays on a filesystem path, not a named pipe."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
engines=(root/"src/engines.cpp").read_text(encoding="utf-8")
header=(root/"src/engines.h").read_text(encoding="utf-8")
single=(root/"src/utils/single_instance.hpp").read_text(encoding="utf-8")

start=engines.index("QString engines::enginePaths::socketPath()")
lock=engines.index("QString engines::enginePaths::socketLockPath()")
confirm=engines.index("void engines::enginePaths::confirmPaths")
socket_body=engines[start:lock]
lock_body=engines[lock:confirm]

assert r'return "\\\\.\\pipe\\MediaDownloaderIPC"' in socket_body
assert "single-instance.lock" in lock_body
assert "m_dataPath" in lock_body
assert r"\\.\pipe" not in lock_body
assert "QString socketLockPath() const" in header
assert "if( !m_localServer.listen( m_info.socketPath ) )" in single
assert 'm_info.socketPath + ".lock"' not in single
assert "details::lockPath( m_info.args,m_info.socketPath )" in single
print("Single-instance lock path policy: PASS")
