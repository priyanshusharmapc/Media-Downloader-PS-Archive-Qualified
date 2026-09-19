"""Regression policy for MDPS-AUDIT2-165 Flatpak M3U metadata."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/settings.cpp").read_text(encoding="utf-8")
start=source.index('auto duration = m_obj.value( "duration" )')
end=source.index('QDesktopServices::openUrl',start)
body=source[start:end]

assert "title.replace( '\\r',' ' )" in body
assert "title.replace( '\\n',' ' )" in body
assert 'aa += "#EXTINF:" + duration + ", " + title + "\\n"' in body
print("Flatpak M3U metadata neutralization policy: PASS")
