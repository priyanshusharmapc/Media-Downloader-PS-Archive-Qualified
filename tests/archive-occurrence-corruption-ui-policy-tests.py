"""Regression policy for AUDIT2-022/023 occurrence identity and corrupt-state UI."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/archive/archivetab.h").read_text(encoding="utf-8")
cpp=(root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")
assert "QString selectedEntryKey() const" in hdr
assert "bool m_stateReadable=true" in hdr
assert "data(Qt::UserRole+1).toString()" in cpp
assert "data(Qt::UserRole).toString()" in cpp
table=cpp[cpp.index("void ArchiveTab::refreshTable()"):cpp.index("void ArchiveTab::refreshDetails()")]
assert "loadPlaylistItems(source.key,&playlistError)" in table
assert "loadCanonicalItems(&canonicalError)" in table
assert "m_stateReadable=false" in table and "m_stateReadable=true" in table
assert "pos->setData(Qt::UserRole,p.entryKey)" in table
assert "pos->setData(Qt::UserRole+1,p.itemKey)" in table
details=cpp[cpp.index("void ArchiveTab::refreshDetails()"):cpp.index("void ArchiveTab::refreshActivity()")]
assert "candidate.entryKey==entryKey" in details
assert "occurrence.itemKey!=itemKey" in details
assert "Archive selection identity mismatch" in details
assert 'object.value("entry_key").toString()' in details
assert "m_stateReadable=false" in details
print("Archive occurrence/corrupt-state UI policy: PASS")
