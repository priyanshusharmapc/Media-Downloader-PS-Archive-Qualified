"""Regression policy for MDPS-AUDIT2-063 subtitle persistence."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/batchdownloader.cpp").read_text(encoding="utf-8")
start=source.index("bool batchdownloader::saveSubtitles( const QString& url")
end=source.index("QString batchdownloader::setSubtitleString",start)
body=source[start:end]

guard=body.index("if( !reply.success() || s.isEmpty() )")
save=body.index("QSaveFile f( e )")
assert guard < save
assert "QIODevice::WriteOnly | QIODevice::Truncate" not in body
assert "f.write( s ) != s.size()" in body
assert "!f.commit()" in body
print("subtitle atomic-save policy: PASS")
