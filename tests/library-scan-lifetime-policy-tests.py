"""Regression policy for MDPS-AUDIT2-085 Library scan lifetime."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/library.h").read_text(encoding="utf-8")
src=(root/"src/library.cpp").read_text(encoding="utf-8")

assert "QPointer" in hdr
assert "std::shared_ptr< std::atomic_bool > m_scanContinue" in hdr
show=src[src.index("void library::showContents"):src.index("void library::moveUp")]
assert "QPointer< library > m_parent" in show
assert "directoryEntries bg()" in show
assert "directoryManager::readAll( m_path,*m_continue )" in show
assert "m_parent->m_directoryEntries = std::move( entries )" in show
assert "!m_parent || !m_continue->load()" in show
assert "m_parent.m_continue" not in show
assert "m_parent.m_directoryEntries" not in show
exit_body=src[src.index("void library::exiting"):src.index("void library::retranslateUi")]
assert "m_scanContinue.reset()" in exit_body
print("Library scan lifetime policy: PASS")
