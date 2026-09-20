"""Regression policy for MDPS-AUDIT2-131 playlist stdout framing offsets."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")
start=source.index("void playlistdownloader::stdOut::parseYtDlpData")
body=source[start:source.index("\n}",start)+2]
assert "line.mid( position,m - position )" in body
assert "position = m + jsonMarker.size()" in body
assert "line.mid( position,m )" not in body
assert "position = position + m + jsonMarker.size()" not in body
assert "data.add( line.mid( position )" in body
print("Playlist framed stdout offset policy: PASS")
