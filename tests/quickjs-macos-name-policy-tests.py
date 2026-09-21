"""Regression policy for MDPS-AUDIT2-081 QuickJS-ng platform mapping parity."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/quickjs_ng.cpp").read_text(encoding="utf-8")

entry_start=source.index("utility::addJsonCmd::entry::args quickjs_ng::entryCmd")
entry_end=source.index("void quickjs_ng::init",entry_start)
entry=source[entry_start:entry_end]
assert 'e == "MacOS"' in entry and 'qjs-darwin' in entry

map_start=source.index("quickjs_ng::nameAndExe quickjs_ng::getNameAndExe()")
map_end=source.index("quickjs_ng::~quickjs_ng",map_start)
mapping=source[map_start:map_end]
assert "utility::platformIsOSX()" in mapping
assert 'return str( "qjs-darwin" )' in mapping
print("QuickJS-ng macOS runtime mapping parity: PASS")
