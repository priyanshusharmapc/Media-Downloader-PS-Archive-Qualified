"""Regression policy for MDPS-AUDIT2-079 restored main-window resizability."""
from __future__ import annotations
import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
source = (p.parse_args().source_root / "src/settings.cpp").read_text(encoding="utf-8")

start = source.index("void settings::setMainWindowDimensions( QWidget * s )")
end = source.index("int settings::tabNumber()", start)
restore = source[start:end]

assert "s->setGeometry" in restore, "persisted geometry must still be restored"
assert "setFixedSize" not in restore, "geometry restore must not lock min/max size"
assert "setMinimumSize" not in restore and "setMaximumSize" not in restore, (
    "saved geometry must not become a resizing constraint")
print("main-window restored geometry remains resizable: PASS")
