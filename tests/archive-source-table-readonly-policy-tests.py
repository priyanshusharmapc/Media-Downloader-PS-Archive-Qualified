"""Regression policy for MDPS-AUDIT2-200 Archive source table read-only state."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")
start=s.index("m_table=new QTableWidget")
end=s.index("rightLayout->addWidget(m_table,3)",start)
body=s[start:end]
assert "m_table->setEditTriggers(QAbstractItemView::NoEditTriggers)" in body
assert "m_table->setSelectionBehavior(QAbstractItemView::SelectRows)" in body
print("Archive source-table read-only policy: PASS")
