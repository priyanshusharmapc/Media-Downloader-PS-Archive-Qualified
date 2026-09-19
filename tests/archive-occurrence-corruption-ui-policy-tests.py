"""Regression policy for MDPS-AUDIT2-022/023 Archive occurrence identity and corrupt-state UI."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/archive/archivetab.h").read_text(encoding="utf-8")
cpp=(root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")

assert "QString selectedEntryKey() const" in hdr
assert "data(Qt::UserRole+1).toString()" in cpp
assert "data(Qt::UserRole).toString()" in cpp
assert "pos->setData(Qt::UserRole,p.entryKey)" in cpp
assert "pos->setData(Qt::UserRole+1,p.itemKey)" in cpp
assert "if(x.entryKey==entryKey)" in cpp

table=cpp[cpp.index("void ArchiveTab::refreshTable()"):cpp.index("void ArchiveTab::refreshDetails()")]
assert "loadPlaylistItems(source.key,&playlistError)" in table
assert "Playlist state error:" in table
assert "m_table->setRowCount(0)" in table

details=cpp[cpp.index("void ArchiveTab::refreshDetails()"):cpp.index("void ArchiveTab::refreshActivity()")]
assert "loadPlaylistItems(source.key,&playlistError)" in details
assert "Archive state error:" in details
assert 'object.value("entry_key").toString()' in details

print("Archive occurrence/corrupt-state UI policy: PASS")
