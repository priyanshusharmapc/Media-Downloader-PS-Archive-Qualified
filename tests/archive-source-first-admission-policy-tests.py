"""Regression policy for MDPS-AUDIT2-196 first-source admission."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/archive/archivetab.h").read_text(encoding="utf-8")
cpp=(root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")

assert "requireCompleteBeforeFirstCommit=false" in hdr
add=cpp[cpp.index("void ArchiveTab::addPlaylist()"):cpp.index("void ArchiveTab::removePlaylist()")]
assert "store.saveSources" not in add
assert "operationScanOrSync({source},true,true)" in add

op=cpp[cpp.index("QString ArchiveTab::operationScanOrSync"):cpp.index("void ArchiveTab::runAsync")]
assert "requireCompleteBeforeFirstCommit&&!snapshot.complete" in op
gate=op.index("requireCompleteBeforeFirstCommit&&!snapshot.complete")
reconcile=op.index("store.reconcile(source,snapshot")
assert gate < reconcile
assert "playlist_admission_rejected" in op
assert "playlist_added" in op

print("Archive first-source admission policy: PASS")
