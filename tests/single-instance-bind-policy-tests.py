"""Regression policy for MDPS-AUDIT2-060 single-instance listener binding."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/utils/single_instance.hpp").read_text(encoding="utf-8")
start=source.index("void start()")
end=source.index("QLocalServer m_localServer",start)
body=source[start:end]

assert "if( !m_localServer.listen( m_info.socketPath ) )" in body
assert "m_info.app.exit( 1 )" in body
assert body.index("m_localServer.listen") < body.index("m_mainApp = std::make_unique")
assert body.index("m_mainApp->start") < body.index("m_lockFile.unlock")
print("Single-instance listener binding policy: PASS")

# The same finding also covers startup-lock acquisition. A non-contention lock
# failure must stop before any listener/GUI startup is attempted.
ctor=source[source.index("oneinstance( AppInfo info"):source.index("~oneinstance()",source.index("oneinstance( AppInfo info"))]
assert "m_lockOwned = m_lockFile.lock()" in ctor
assert "if( !m_lockOwned )" in ctor
assert "m_info.app.exit( 1 )" in ctor
assert "bool m_lockOwned = false" in source
assert "if( !m_lockOwned )return" in body
