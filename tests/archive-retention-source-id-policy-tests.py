"""Regression policy for MDPS-AUDIT2-183/198 Archive retention and source identity."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
tab=(root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")

add=tab[tab.index("void ArchiveTab::addPlaylist()"):tab.index("void ArchiveTab::removePlaylist()")]
assert "const auto key=archive::sourceKeyFromUrl(url)" in add
assert "source.key=key" in add
assert "currentMSecsSinceEpoch" not in add

remove=tab[tab.index("void ArchiveTab::removePlaylist()"):tab.index("void ArchiveTab::browseRoot()",tab.index("void ArchiveTab::removePlaylist()")) if "void ArchiveTab::browseRoot()" in tab[tab.index("void ArchiveTab::removePlaylist()"):] else tab.index("void ArchiveTab::scanSelected()",tab.index("void ArchiveTab::removePlaylist()"))]
assert "sources.remove(i)" in remove
assert "store.saveSources" in remove
for forbidden in ["removeCanonicalItemFiles","removePlaylist(","QFile::remove","removeRecursively"]:
    assert forbidden not in remove.replace("void ArchiveTab::removePlaylist()","")
assert "will not be deleted" in remove

print("Archive retention/source identity policy: PASS")
