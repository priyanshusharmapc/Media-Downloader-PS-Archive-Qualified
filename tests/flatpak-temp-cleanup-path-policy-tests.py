"""Regression policy for MDPS-AUDIT2-174 Flatpak temp cleanup path."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/settings.cpp").read_text(encoding="utf-8")
start=s.index("void settings::clearFlatPakTemps"); end=s.index("QString settings::windowsDimensions",start)
body=s[start:end]
assert 'QDir( m_appDataPath ).filePath( "tmp" )' in body
assert 'm_appDataPath + "tmp"' not in body
print("Flatpak temp cleanup path policy: PASS")
