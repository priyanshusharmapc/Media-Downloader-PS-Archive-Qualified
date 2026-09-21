"""Regression policy for MDPS-AUDIT2-175 custom plugin admission without persistence."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines.cpp").read_text(encoding="utf-8")
start=source.index("QString engines::addEngine( const QByteArray& data")
end=source.index("void engines::removeEngine",start)
body=source[start:end]

assert "auto candidate =" in body
assert "candidate->exePath().isEmpty()" in body
assert "Rejected engine definition before persistence" in body
assert body.index("candidate->exePath().isEmpty()") < body.index("QSaveFile file( path )")
assert "file.write( data ) != data.size()" in body
assert "!file.commit()" in body
assert "engineAdd( extensionFileName,candidate.move(),id )" in body

# The shared installation function is also the automatic definition refresh
# path. A plugin fix must not make all same-name refreshes impossible.
assert "Plugin definition already exists" not in body
assert "QFileInfo::exists( path )" not in body
assert "QIODevice::WriteOnly | QIODevice::Truncate" not in body

print("Custom plugin pre-admission preservation policy: PASS")
