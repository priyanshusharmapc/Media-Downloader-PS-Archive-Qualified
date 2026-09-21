"""Regression policy for MDPS-AUDIT2-139 signed LogWindow geometry."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/logwindow.cpp").read_text(encoding="utf-8")
h0=s.index("void logWindow::Hide"); h1=s.index("void logWindow::Show",h0)
show1=s.index("void logWindow::clear",h1)
hide=s[h0:h1]; show=s[h1:show1]
assert 'x + " " + y + " " + w + " " + h' in hide
assert "util::split( w,' ',true )" in show
assert "toInt( &xOk )" in show and "toInt( &yOk )" in show
assert "toInt( &widthOk )" in show and "toInt( &heightOk )" in show
assert "width > 0 && height > 0" in show
assert 'x + "-" + y' not in hide
print("Signed LogWindow geometry policy: PASS")
