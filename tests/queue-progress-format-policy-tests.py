"""Regression policy for MDPS-AUDIT2-140 aggregate queue progress."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/tableWidget.cpp").read_text(encoding="utf-8")
start=s.index("QString tableWidget::completeProgress"); end=s.index("tableWidget::tableWidget",start)
body=s[start:end]
assert "rowCount > 0 ? z * 100 / rowCount : 0" in body
assert "rowCount > 0 && z == rowCount" in body
assert 'a.startsWith( "100" )' not in body
print("Aggregate queue progress policy: PASS")
