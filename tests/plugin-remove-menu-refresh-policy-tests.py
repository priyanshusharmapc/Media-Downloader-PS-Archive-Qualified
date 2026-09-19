"""Regression policy for MDPS-AUDIT2-123 plugin remove-menu refresh."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/configure.cpp").read_text(encoding="utf-8")
r0=s.index("QMenu * configure::removeExtenion"); r1=s.index("void configure::updateExtensionsRemoveList",r0)
a0=s.index("void configure::addEngine"); a1=s.index("void configure::saveOptions",a0)
assert "this->updateExtensionsRemoveList()" in s[r0:r1]
assert "this->updateExtensionsRemoveList()" in s[a0:a1]
print("Plugin remove-menu refresh policy: PASS")
