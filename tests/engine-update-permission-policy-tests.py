"""Regression policy for MDPS-AUDIT2-041 promoted engine permissions."""
from __future__ import annotations
import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
root = p.parse_args().source_root
source = (root / "src/networkAccess.cpp").read_text(encoding="utf-8")

needle = "auto m = opts.file.rename( opts.exeBinPath )"
start = source.index(needle)
end = source.index("}else{", start)
success = source[start:end]

assert "utility::setPermissions( opts.exeBinPath )" in success, success
assert "utility::setPermissions( opts.file.src() )" not in success, success
print("promoted engine permission target policy: PASS")
