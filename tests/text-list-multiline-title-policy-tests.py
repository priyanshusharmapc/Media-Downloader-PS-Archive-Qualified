"""Regression policy for MDPS-AUDIT2-145 TXT saved-list multiline titles."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/utility.cpp").read_text(encoding="utf-8")
start=source.index("void utility::saveDownloadList( const Context& ctx,QMenu&")
end=source.index("bool utility::isRelativePath",start)
body=source[start:end]
assert "title.split( '\\n',Qt::KeepEmptyParts )" in body
assert 'm.append( "#" + line.toUtf8() + "\\n" )' in body
assert 'm.append( "#" + title + "\\n" + url' not in body
print("TXT saved-list multiline-title policy: PASS")
