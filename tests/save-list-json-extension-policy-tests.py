"""Regression policy for MDPS-AUDIT2-113 saved-list JSON extension."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/utility.cpp").read_text(encoding="utf-8")
start=s.index("void utility::saveDownloadList( const Context& ctx,QMenu&")
end=s.index("bool utility::isRelativePath",start)
body=s[start:end]
assert 'QFileInfo( s ).suffix().compare( "json",Qt::CaseInsensitive ) == 0' in body
assert 'endsWith( ".json" )' not in body
print("Saved-list JSON extension policy: PASS")
