"""Regression for MDPS-AUDIT2-154 empty table-filter values."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/tableWidget.h").read_text(encoding="utf-8")
start=source.index("void filterTable")
end=source.index("void arrangeTable",start)
body=source[start:end]

assert "if( !m.trimmed().isEmpty() && !l.contains( m ) )" in body
loop=body.index("for( const auto& it : l )")
guard=body.index("if( it.trimmed().isEmpty() )",loop)
indexing=body.index("s[ 0 ] = s[ 0 ].toUpper()",guard)
assert loop < guard < indexing
print("table empty-filter policy: PASS")
