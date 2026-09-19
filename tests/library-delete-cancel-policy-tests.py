"""Regression policy for MDPS-AUDIT2-117 Library batch cancellation."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
library=(root/"src/library.cpp").read_text(encoding="utf-8")
entries=(root/"src/directoryEntries.cpp").read_text(encoding="utf-8")

delete_start=library.index("bool library::deletePath")
delete_end=library.index("void library::setRenameUiVisible",delete_start)
delete_body=library[delete_start:delete_end]
assert "if( !m_continue )" in delete_body
assert "if( !m_continue || items.empty() )" in delete_body
assert "if( !m_parent.m_continue )" in delete_body

# New operations explicitly arm cancellation once.
assert library.count("m_continue = true ;") >= 4

# Per-directory workers must never reset the shared operation token.
assert "m_continue = true ;" not in entries.split("#ifdef Q_OS_WIN",1)[1].split("#else",1)[0].split("private:",1)[0]
posix=entries.split("#else",1)[1].split("#endif",1)[0]
assert "m_continue = true ;" not in posix.split("private:",1)[0]

# Standalone enumeration still initializes its own token.
assert "std::atomic_bool s{ true }" in entries
print("Library delete cancellation scope policy: PASS")
