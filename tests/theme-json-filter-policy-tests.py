"""Regression policy for MDPS-AUDIT2-084 theme discovery filtering."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/themes.cpp").read_text(encoding="utf-8")

start=source.index("void themes::updateThemes()")
end=source.index("int themes::indexAt",start)
body=source[start:end]
assert 'entryList( { "*.json" },QDir::Filter::Files )' in body
assert "it.chop( 5 )" in body
assert 'it.replace( ".json","" )' not in body
print("theme discovery JSON-only filtering: PASS")
