"""Regression policy for MDPS-AUDIT2-111 ExtendedSelection modifiers."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/tableWidget.cpp").read_text(encoding="utf-8")
start=s.index("void tableWidget::selectRow")
end=s.index("void tableWidget::clear",start)
body=s[start:end]
assert "Qt::ControlModifier | Qt::ShiftModifier | Qt::MetaModifier" in body
assert "keyboardModifiers() != Qt::ControlModifier" not in body
assert "if( !( modifiers &" in body
print("Extended selection modifier policy: PASS")
