"""Regression policy for Library destructive-operation cancellation and AUDIT2-085 lifetime."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/library.h").read_text(encoding="utf-8")
src=(root/"src/library.cpp").read_text(encoding="utf-8")
entries=(root/"src/directoryEntries.cpp").read_text(encoding="utf-8")

assert "std::shared_ptr< std::atomic_bool > m_deleteContinue" in hdr
ctor=src[src.index("library::library( const Context& ctx )"):src.index("void library::moveUp")]
assert "if( m_deleteContinue )" in ctor
assert "*m_deleteContinue = false" in ctor

delete=src[src.index("void library::deleteEntries"):src.index("void library::setRenameUiVisible")]
assert "m_deleteContinue->load()" in delete
assert "auto keepGoing = m_deleteContinue" in delete
assert "deleteLibraryPath( m_root,m_path,*m_continue )" in delete
assert "utils::qthread::run( this,meaw" in delete
bg=delete[delete.index("bool bg()"):delete.index("void fg(")]
assert "m_parent" not in bg

delete_all=src[src.index("void library::deleteAll"):src.index("void library::enableAll")]
assert "auto keepGoing = m_deleteContinue" in delete_all
assert "directoryManager::removeDirectoryContents( m_path,*m_continue )" in delete_all
assert "utils::qthread::run( this,meaw" in delete_all
bg_all=delete_all[delete_all.index("void bg()"):delete_all.index("void fg()")]
assert "m_parent" not in bg_all

exit_body=src[src.index("void library::exiting"):src.index("void library::retranslateUi")]
assert "*m_deleteContinue = false" in exit_body
assert "m_deleteContinue.reset()" in exit_body

# Recursive directory helpers consume the caller-owned atomic and never re-arm it.
assert "std::atomic_bool& m_continue" in entries
print("Library delete cancellation/lifetime policy: PASS")
