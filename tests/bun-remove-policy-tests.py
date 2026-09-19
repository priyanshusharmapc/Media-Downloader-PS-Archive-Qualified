"""Regression for MDPS-AUDIT2-162 Bun removal filename parity."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/bun.cpp").read_text(encoding="utf-8")

remove_start=source.index("void bun::remove")
remove=source[remove_start:]
assert 'utility::platformIsWindows() ? "bun.exe" : "bun"' in remove
assert 'enginePath.binPath( "bun" )' not in remove

entry_start=source.index("utility::addJsonCmd::entry::args bun::entryCmd")
entry_end=source.index("void bun::init",entry_start)
entry=source[entry_start:entry_end]
assert 'data.emplace_back( "x86","bun.exe" )' in entry
assert 'data.emplace_back( "amd64","bun.exe" )' in entry
print("Bun removal filename parity policy: PASS")
