"""Regression policy for MDPS-AUDIT2-088 UI scale lower bound."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
c=(root/"src/configure.cpp").read_text(encoding="utf-8")
s=(root/"src/settings.cpp").read_text(encoding="utf-8")
start=c.index("class scaleUi"); end=c.index("pbConfigureScaleDown",start)
body=c[start:end]
assert "minimumScaleFactor = 0.05" in body
assert "!std::isfinite( interval ) || interval <= 0.0" in body
assert "!std::isfinite( s ) || s < minimumScaleFactor" in body
gstart=s.index("double settings::highDpiScalingFactorValue")
gend=s.index("double settings::highDpiScalingFactorInterval",gstart)
g=s[gstart:gend]
assert "!std::isfinite( m ) || m < minimumScaleFactor ? minimumScaleFactor : m" in g
assert "m == 0.0" not in g
print("UI scale minimum policy: PASS")

assert "#include <cmath>" in c
assert "#include <cmath>" in s
