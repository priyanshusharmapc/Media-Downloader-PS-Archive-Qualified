"""Regression policy for MDPS-AUDIT2-072 Library confirmation target binding."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/library.cpp").read_text(encoding="utf-8")

ctor=source[source.index("library::library( const Context& ctx )"):source.index("void library::moveUp()")]
confirm=ctor[ctor.index("connect( m_ui.pbLibrarySetNewFileName"):ctor.index("connect( this,&library::addEntrySignal")]
assert "m_table.currentRow()" not in confirm
assert "m_table.selectedRows()" not in confirm
assert "const auto rows = this->pendingRows()" in confirm
assert "directoryMatches" in confirm
assert "expectedCount == 1 && rows.size() == 1" in confirm
assert "this->clearPendingAction()" in confirm

menu=source[source.index("void library::cxMenuRequested"):source.index("void library::arrangeAndShow")]
assert "this->capturePendingRow( row )" in menu
assert "this->capturePendingRows( m_table.selectedRows() )" in menu
assert "this->capturePendingDirectory()" in menu

helpers=source[source.index("void library::capturePendingRows"):source.index("bool library::hasMultipleSelections")]
assert "m_pendingActionDirectory = m_currentPath" in helpers
assert "m_table.item( row,1 ).text() == name" in helpers
assert "rows.clear()" in helpers

print("Library pending-target binding policy: PASS")
