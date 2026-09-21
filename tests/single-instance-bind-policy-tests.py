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
assert body.index("m_mainApp->start") < body.index("Keep the ownership lock for the full primary lifetime")
assert "m_lockFile.unlock" not in body
print("Single-instance listener binding policy: PASS")

ctor=source[source.index("oneinstance( AppInfo info"):source.index("~oneinstance()",source.index("oneinstance( AppInfo info"))]
assert "m_lockOwned = m_lockFile.tryLock( 0 )" in ctor
assert "bool m_lockOwned = false" in source
assert "staleEndpointError" in source
assert "if( !this->staleEndpointError( error ) || !m_lockFile.tryLock( 0 ) )" in source
assert "refusing stale cleanup" in source
