"""Regression policy for MDPS-AUDIT2-161 Deno Windows ARM64 selection."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/deno.cpp").read_text(encoding="utf-8")
entry=source[source.index("deno::entryCmd"):source.index("void deno::init")]
assert 'data.emplace_back( "aarch64","deno.exe" )' in entry
sel=source[source.index("QString deno::urlFileName"):source.index("bool deno::autoUpdate")]
assert "cpu.aarch64()" in sel
assert 'return "deno-aarch64-pc-windows-msvc.zip" ;' in sel
print("Deno Windows ARM64 selection policy: PASS")
