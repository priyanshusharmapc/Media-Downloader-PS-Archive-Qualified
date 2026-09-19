"""Regression policy for MDPS-AUDIT2-091 SafariBooks slash-only input."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/engines/safaribooks.cpp").read_text(encoding="utf-8")
start=s.index("void safaribooks::updateDownLoadCmdOptions")
body=s[start:]
assert "const auto m = util::split" in body
assert "if( m.isEmpty() )" in body
assert body.index("if( m.isEmpty() )") < body.index("m.last()")
assert "s.urls.clear()" in body
print("SafariBooks slash-only guard policy: PASS")
