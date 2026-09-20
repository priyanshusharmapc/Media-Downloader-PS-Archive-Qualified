"""Regression policy for MDPS-AUDIT2-166 Flatpak title sentinel."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/settings.cpp").read_text(encoding="utf-8")
start=s.index("void settings::mediaPlayer::action::operator()"); end=s.index("void settings::sLogger::add",start)
body=s[start:end]
assert 'title != "NA"' in body
assert "!title.isEmpty()" in body
assert 'title.contains( "NA" )' not in body
print("Flatpak title sentinel policy: PASS")
