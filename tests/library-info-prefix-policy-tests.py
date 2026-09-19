"""Regression policy for MDPS-AUDIT2-110 info_ Library visibility."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/directoryEntries.cpp").read_text(encoding="utf-8")
start=s.index("bool directoryEntries::valid( const char * e )")
end=s.index("directoryEntries directoryManager::readAll",start)
body=s[start:end]
assert 'strcmp( e,".." )' in body and 'strcmp( e,"." )' in body
assert 'wcscmp( s,L".." )' in body and 'wcscmp( s,L"." )' in body
assert 'strncmp( e,"info_",5 )' not in body
assert 'wcsncmp( s,L"info_",5 )' not in body
print("Library info_ visibility policy: PASS")
