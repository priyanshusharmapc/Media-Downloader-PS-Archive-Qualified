"""Regression policy for MDPS-AUDIT2-157 Lux ARM64 selection."""
from __future__ import annotations
import argparse,json
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
source=(root/"src/engines/lux.cpp").read_text(encoding="utf-8")
meta=json.loads((root/"extensions/lux.json").read_text(encoding="utf-8"))
start=source.index("bool lux::foundNetworkUrl")
end=source.index("static QByteArray _hash",start)
body=source[start:end]
for asset in ["Windows_arm64","Linux_arm64","Darwin_arm64.tar.gz"]:
    assert asset in body
assert "cpu.aarch64()" in body
assert "aarch64" in meta["Cmd"]["Generic"]
assert "aarch64" in meta["Cmd"]["Windows"]
print("Lux ARM64 selection policy: PASS")
