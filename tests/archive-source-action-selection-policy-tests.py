"""Regression policy for MDPS-AUDIT2-197 Archive source action gating."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
cpp=(root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")
hdr=(root/"src/archive/archivetab.h").read_text(encoding="utf-8")

assert "QAction* m_openPlaylistAction" in hdr
start=cpp.index("void ArchiveTab::updateActionState()")
end=cpp.index("void ArchiveTab::setBusy",start)
body=cpp[start:end]
assert "sourceSelected=ready&&!selectedSourceKey().isEmpty()" in body
for name in ["m_remove","m_scan","m_syncSelected","m_retry"]:
    assert name in body
assert "m_openPlaylistAction->setEnabled(sourceSelected)" in body
assert "m_syncAll" in body and "globalControls" in body

wire=cpp[cpp.index("void ArchiveTab::wireUi()"):cpp.index("QString ArchiveTab::configuredRoot")]
assert "currentRowChanged" in wire and "updateActionState()" in wire

refresh=cpp[cpp.index("void ArchiveTab::refreshSources()"):cpp.index("archive::Source ArchiveTab::selectedSource()")]
assert "updateActionState()" in refresh
print("Archive source action selection policy: PASS")
