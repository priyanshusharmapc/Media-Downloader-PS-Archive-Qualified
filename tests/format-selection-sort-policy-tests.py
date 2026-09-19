"""Regression policy for MDPS-AUDIT2-155 format selection across sort."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/tableWidget.h").read_text(encoding="utf-8")
assert "#include <QSignalBlocker>" in s
start=s.index("void arrangeTable( bool ascending,int column )")
end=s.index("template< typename Rows >",start)
body=s[start:end]
assert "QStringList selectedIds" in body
assert "this->isSelected( row )" in body
assert "QSignalBlocker blocker( m_table )" in body
assert "selectedIds.contains( m_table.item( row,0 )->text() )" in body
assert "setSelected( true )" in body
print("Format selection preservation policy: PASS")
