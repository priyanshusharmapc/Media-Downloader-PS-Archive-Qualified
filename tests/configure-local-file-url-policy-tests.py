"""Regression policy for MDPS-AUDIT2-087 local folder URLs."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/configure.cpp").read_text(encoding="utf-8")
start=s.index("pbOpenThemeFolder")
end=s.index("class scaleUi",start)
body=s[start:end]
assert body.count("m_settings.openUrl") >= 3
assert '"file:///" +' not in body
print("Configure local-file URL policy: PASS")
