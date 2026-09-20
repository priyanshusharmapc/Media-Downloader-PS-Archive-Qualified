"""Regression policy for MDPS-AUDIT2-158 global Library sorting."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/directoryEntries.h").read_text(encoding="utf-8")
sort0=s.index("template< typename sorter>"); join0=s.index("void join( bool folderFirst )",sort0)
sort=s[sort0:join0]
join=s[join0:s.index("directoryEntries::iter Iter",join0)]
assert "std::sort( m_joined.begin(),m_joined.end()" in sort
assert "m_globalJoined = m_joined" in sort
assert "if( folderFirst )" in join
assert "else{" in join
assert "m_joined = m_globalJoined" in join
assert "m_files" not in join.split("if( folderFirst )",1)[0]
print("Library global mixed-sort policy: PASS")

assert "std::vector< directoryEntries::wrapper > m_globalJoined" in s
assert "m_globalJoined.clear()" in s
