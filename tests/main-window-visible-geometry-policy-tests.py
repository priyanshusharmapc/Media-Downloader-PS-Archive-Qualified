"""Regression policy for MDPS-AUDIT2-177 main-window geometry visibility."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/settings.cpp").read_text(encoding="utf-8")
start=s.index("void settings::setMainWindowDimensions"); end=s.index("int settings::tabNumber",start)
body=s[start:end]
assert "QGuiApplication::screens()" in body
assert "availableGeometry().intersected( restored )" in body
assert "QGuiApplication::primaryScreen()" in body
assert "restored.moveCenter( available.center() )" in body
assert "s->setGeometry( restored )" in body
print("Main-window visible geometry policy: PASS")
