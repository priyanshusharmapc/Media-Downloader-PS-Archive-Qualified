"""Regression policy for MDPS-AUDIT2-083 yt-dlp protocol/container labels."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/yt-dlp.cpp").read_text(encoding="utf-8")

assert 'QString( "Proto: %1\\ncontainer: %2\\n" )' in source
assert 'QString( "Proto: %1%2\\ncontainer: %2\\n" )' not in source
print("yt-dlp protocol/container display labels: PASS")
