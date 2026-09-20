"""Regression policy for MDPS-AUDIT2-120 exact history URL deduplication."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/utility.cpp").read_text(encoding="utf-8")
start=s.index("bool historyFound"); end=s.index("void updateHistory",start)
body=s[start:end]
assert 'value.toObject().value( "Url" ).toString() == url' in body
assert "s.contains( url.toUtf8() )" not in body
assert "QJsonDocument::fromJson" in body
print("Exact download-history URL policy: PASS")
