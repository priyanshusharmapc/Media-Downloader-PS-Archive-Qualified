"""Regression policy for MDPS-AUDIT2-106 current-column tracking."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
s=(root/"src/tableWidget.h").read_text(encoding="utf-8")
c=(root/"src/configure.cpp").read_text(encoding="utf-8")
start=s.index("void selectRow( QTableWidgetItem * current,QTableWidgetItem * previous,int s )")
end=s.index("bool isSelected",start)
body=s[start:end]
assert "current ? current->column() : -1" in body
assert "previous->column()" not in body
print("Mini-table current-column policy: PASS")

menu=c[c.index("m_tablePresetOptions.connect( &QTableWidget::customContextMenuRequested"):c.index("connect( m_ui.cbConfigureEngines",c.index("m_tablePresetOptions.connect( &QTableWidget::customContextMenuRequested"))]
assert "QPoint point" in menu
assert "itemAt( point )" in menu
assert "setCurrentItem( target )" in menu
assert menu.index("setCurrentItem( target )") < menu.index('tr( "Copy" )')
