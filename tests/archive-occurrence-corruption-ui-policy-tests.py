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

# 022: selected occurrence identity and canonical identity must remain coupled.
assert "p.itemKey!=key" in details
assert "entryKey.isEmpty()" in details and "key.isEmpty()" in details
assert "Archive selection identity mismatch" in details

# 023: corrupt durable state disables Archive mutations and clears stale detail
# panes until a later successful load proves state readable again.
assert "bool m_stateReadable=true" in hdr
action=cpp[cpp.index("void ArchiveTab::updateActionState()"):cpp.index("void ArchiveTab::setBusy")]
assert "m_stateReadable" in action
assert "m_stateReadable=false" in table
assert "m_stateReadable=true" in table
assert "m_sourceDetails->clear()" in table
