"""Regression policy for MDPS-AUDIT2-164 Library date semantics."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/directoryEntries.cpp").read_text(encoding="utf-8")
assert "ftLastWriteTime.dwLowDateTime" in s
assert "ftLastWriteTime.dwHighDateTime" in s
assert "ftCreationTime.dwLowDateTime" not in s
assert "m.st_mtime" in s
assert "m.st_ctime" not in s
print("Library modification-time sorting policy: PASS")
