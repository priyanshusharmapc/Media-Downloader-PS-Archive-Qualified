"""Regression policy for MDPS-AUDIT2-167 theme stylesheet replacement."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/themes.cpp").read_text(encoding="utf-8")
needle='auto s = obj.value( "QToolTipStyleSheet" ).toString()'
start=s.index(needle); end=s.index("themes::JObject themes::baseTheme",start)
body=s[start:end]
assert "app.setStyleSheet( s )" in body
assert "if( !s.isEmpty() )" not in body
print("Theme stylesheet replacement policy: PASS")
