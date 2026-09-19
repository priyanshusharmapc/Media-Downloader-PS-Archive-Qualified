"""Regression policy for MDPS-AUDIT2-059 Windows installer shortcut icons."""
from __future__ import annotations
import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
root = p.parse_args().source_root
cm = (root / "CMakeLists.txt").read_text(encoding="utf-8")

quick = [line for line in cm.splitlines() if "Quick Launch" in line and "IconFilename:" in line]
assert len(quick) == 2, f"expected Qt5 and Qt6 Quick Launch entries, got {len(quick)}"
for line in quick:
    assert "media-downloader.ico" in line, line
    assert "media-downloade.ico" not in line, line

installed = [line for line in cm.splitlines() if "media-downloader.ico" in line and ("file( COPY" in line or "Source:" in line)]
assert installed, "installer policy must still install media-downloader.ico"
print("Windows installer shortcut icon policy: PASS")
