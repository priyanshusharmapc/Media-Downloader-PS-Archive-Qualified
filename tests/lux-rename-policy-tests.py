"""Regression policy for MDPS-AUDIT2-107 Lux rename publication."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/engines/lux.cpp").read_text(encoding="utf-8")
start=s.index("const QByteArray& lux::lux_dlFilter::setFileName")
end=s.index("QByteArray lux::lux_dlFilter::fileNameFromCmd",start)
body=s[start:end]
assert "utils::qthread::run" not in body
assert "else if( old == New || QFile::rename( old,New ) )" in body
assert body.index("old == New || QFile::rename( old,New )") < body.index("e.addFileName( fileNameCmd )")
assert 'e.addFileName( fileName )' in body
assert "ERROR: Failed To Rename Downloaded File" in body
print("Lux rename transaction policy: PASS")
