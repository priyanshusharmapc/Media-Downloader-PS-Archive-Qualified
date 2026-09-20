"""Regression for MDPS-AUDIT2-108 yt-dlp archive migration fallback."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines.h").read_text(encoding="utf-8")
start=source.index("QString archiveFilePathByName")
end=source.index("QString add(",start)
body=source[start:end]

current=body.index("if( QFile::exists( current ) )")
legacy=body.index("if( QFile::exists( legacy ) )",current)
rename=body.index("if( QFile::rename( legacy,current ) )",legacy)
race=body.index("if( QFile::exists( current ) )",rename)
fallback=body.index("return legacy",race)
assert current < legacy < rename < race < fallback
assert "QFile::rename( m,o )" not in body
print("yt-dlp archive migration fallback policy: PASS")
