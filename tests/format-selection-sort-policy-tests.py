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
assert "struct sortableItem" in body
assert "bool wasSelected" in body
assert "this->isSelected( static_cast< int >( row ) )" in body
assert "QSignalBlocker blocker( m_table )" in body
assert "if( item.wasSelected )" in body
assert "selectedIds.contains" not in body
assert "setSelected( true )" in body
print("Format selection preservation policy: PASS")

assert "std::sort( rows.begin(),rows.end()" in body
assert "comparer( a.stuff,b.stuff )" in body
