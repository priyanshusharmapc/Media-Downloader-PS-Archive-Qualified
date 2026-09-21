"""Regression policy for MDPS-AUDIT2-082 svtplay-dl exact-three-token parsing."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/svtplay-dl.cpp").read_text(encoding="utf-8")

start=source.index("}else if( n == 3 ){")
end=source.index("\n\t\t}",start)+4
body=source[start:end]
assert body.count("a.takeAt( 0 )") == 3, (
    "three input tokens may be consumed at most three times")
assert 'auto codec      = QString( "N/A" )' in body
assert 'auto resolution = "N/A"' in body
print("svtplay-dl exact-three-token parser bounds: PASS")
