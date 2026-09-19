"""Regression policy for MDPS-AUDIT2-180 Archive runtime localization."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/archive/archivetab.h").read_text(encoding="utf-8")
cpp=(root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")

for member in ["m_rootTitle","m_systemGroup","m_operationGroup","m_playlistLabel","m_openRootAction","m_openPlaylistAction","m_openCatalogAction","m_openMissingAction","m_importsAction","m_openLogsAction"]:
    assert member in hdr

start=cpp.index("void ArchiveTab::retranslateUi()")
end=cpp.index("void ArchiveTab::tabEntered()",start)
body=cpp[start:end]
for text in ["Archive root:","SYSTEM HEALTH","Add Playlist","Sync Selected","Search archive items","Availability","Recovery","Open Archive Folder","Process External Imports"]:
    assert text in body
assert "m_filter->setItemText" in body
assert "m_table->setHorizontalHeaderLabels" in body
assert "m_detailsTabs->setTabText" in body
assert "m_hostTabs.setTabText" in body

print("Archive runtime retranslation policy: PASS")
