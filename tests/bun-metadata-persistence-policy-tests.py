"""Regression policy for MDPS-AUDIT2-163 Bun metadata persistence."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/engines/bun.cpp").read_text(encoding="utf-8")
start=s.index("void bun::init"); end=s.index("void bun::remove",start)
body=s[start:end]
assert 'QFile::exists( m )' in body
assert body.index("QFile::exists( m )") < body.index("QJsonObject mainObj")
assert "return ;" in body
print("Bun metadata persistence policy: PASS")
