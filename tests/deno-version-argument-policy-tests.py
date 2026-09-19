"""Regression for MDPS-AUDIT2-090 Deno regenerated version contract."""
from __future__ import annotations
import argparse
import json
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
source=(root/"src/engines/deno.cpp").read_text(encoding="utf-8")
shipped=json.loads((root/"extensions/deno.json").read_text(encoding="utf-8"))

assert shipped["VersionArgument"] == "--version"
assert 'cmd.start( m,{ "--version" } )' in source
assert 'mainObj.insert( "VersionArgument","--version" )' in source
assert '"-version"' not in source
print("Deno version-argument parity policy: PASS")
