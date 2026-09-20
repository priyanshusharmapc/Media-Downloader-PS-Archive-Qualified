"""Regression policy for MDPS-AUDIT2-137 Library tab lifecycle."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/library.cpp").read_text(encoding="utf-8")
start=s.index("void library::tabEntered"); end=s.index("void library::tabExited",start)
body=s[start:end]
assert "m_settings.enableLibraryTab()" in body
assert "showContents( m_currentPath )" in body
assert "m_table.rowCount() == 0" not in body
print("Library tab re-entry reload policy: PASS")
