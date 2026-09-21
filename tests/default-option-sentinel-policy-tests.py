"""Regression policy for MDPS-AUDIT2-171 standalone default option sentinel."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/utility.cpp").read_text(encoding="utf-8")
start=source.index("QStringList utility::args::parseOptions")
end=source.index("utility::args::args(",start)
body=source[start:end]
assert 'm[ 0 ].compare( "default",Qt::CaseInsensitive )' in body
assert "m.removeFirst()" in body
assert "if( m.isEmpty() )" in body
assert "return {} ;" in body
# Explicit q + default is still suppressed by the existing value-path comparison.
assert 'ss.compare( "default",Qt::CaseInsensitive )' in body
print("Standalone default option sentinel policy: PASS")
