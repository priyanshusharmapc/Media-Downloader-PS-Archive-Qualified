"""Regression policy for MDPS-AUDIT2-136 Library cache coherence."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/library.cpp").read_text(encoding="utf-8")
d0=s.index("void library::deleteEntries"); d1=s.index("void library::setRenameUiVisible",d0)
delete=s[d0:d1]
assert "items.empty()" in delete
assert "showContents( m_currentPath,m_currentNativePath )" in delete
r0=s.index("void library::renameFile"); r1=s.index("void library::keyPressed",r0)
rename=s[r0:r1]
assert "renameEntryNative" in rename
assert "showContents( m_currentPath,m_currentNativePath )" in rename
print("Library mutation cache refresh policy: PASS")
