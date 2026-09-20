"""Regression policy for MDPS-AUDIT2-054 Library filesystem ownership boundary."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
entries=(root/"src/directoryEntries.cpp").read_text(encoding="utf-8")
library=(root/"src/library.cpp").read_text(encoding="utf-8")

# POSIX enumeration must preserve symlink identity instead of following it.
posix=entries.split("#else",1)[1].split("#endif",1)[0]
assert "lstat( s.data(),&m )" in posix
assert "stat( s.data(),&m )" not in posix.replace("lstat( s.data(),&m )","")

# Windows reparse points must not appear as navigable owned folders.
win=entries.split("#ifdef Q_OS_WIN",1)[1].split("#else",1)[0]
add=win[win.index("void add( directoryEntries& entries"):win.index("bool read(",win.index("void add( directoryEntries& entries"))]
assert "FILE_ATTRIBUTE_REPARSE_POINT" in add
assert "return ;" in add

# Library navigation and destructive paths independently prove canonical
# containment within the configured download root.
assert "bool pathWithinLibraryRoot" in library
assert "canonicalFilePath()" in library
ctor=library[library.index("library::library( const Context& ctx )"):library.index("void library::moveUp")]
assert "const auto candidate = QDir::cleanPath" in ctor
assert "if( !pathWithinLibraryRoot( m_downloadFolder,candidate ) )" in ctor

show=library[library.index("void library::showContents"): ]
assert "if( !pathWithinLibraryRoot( m_downloadFolder,safePath ) )" in show

delete_helper=library[library.index("bool deleteLibraryPath"):library.index("library::library(")]
assert "info.isSymLink()" in delete_helper
assert "parentWithinLibraryRoot( root,path )" in delete_helper
assert "pathWithinLibraryRoot( root,path )" in delete_helper

rename=library[library.index("void library::renameFile"):library.index("void library::keyPressed")]
assert "pathWithinLibraryRoot( m_downloadFolder,m_currentPath )" in rename

print("Library filesystem ownership boundary policy: PASS")
