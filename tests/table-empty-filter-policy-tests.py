from pathlib import Path
import argparse
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/tableWidget.h").read_text(encoding="utf-8")
b=s[s.index("void filterTable"):s.index("void arrangeTable",s.index("void filterTable"))]
assert "if( it.trimmed().isEmpty() )" in b
assert "continue" in b
assert b.index("if( it.trimmed().isEmpty() )") < b.index("s[ 0 ] = s[ 0 ].toUpper()")
print("table empty and whitespace-filter policy: PASS")
