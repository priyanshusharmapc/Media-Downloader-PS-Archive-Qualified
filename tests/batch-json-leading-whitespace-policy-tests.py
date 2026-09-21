"""Regression policy for MDPS-AUDIT2-124 JSON list detection."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/batchdownloader.cpp").read_text(encoding="utf-8")
start=s.index("void batchdownloader::getListFromFile( const QString& e")
end=s.index("void batchdownloader::getListFromFile( QMenu&",start)
body=s[start:end]
assert "auto jsonCandidate = list.trimmed()" in body
assert 'jsonCandidate.startsWith( "\\xEF\\xBB\\xBF" )' in body
assert "parseDataFromFile( items,jsonCandidate )" in body
assert "if( list.startsWith( '[' )" not in body
print("Batch JSON leading-whitespace policy: PASS")
