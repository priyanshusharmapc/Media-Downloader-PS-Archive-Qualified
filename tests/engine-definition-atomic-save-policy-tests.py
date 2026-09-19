"""Regression for MDPS-AUDIT2-043 engine-definition atomic replacement."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines.cpp").read_text(encoding="utf-8")
start=source.index("QString engines::addEngine( const QByteArray&")
end=source.index("void engines::removeEngine",start)
body=source[start:end]

assert "QSaveFile f( e )" in body
assert "f.open( QIODevice::WriteOnly )" in body
assert "f.write( data ) == data.size()" in body
assert "f.commit()" in body
assert "f.cancelWriting()" in body
assert "QIODevice::Truncate" not in body
assert "utility::waitForOneSecond()" not in body
print("engine definition atomic replacement policy: PASS")
