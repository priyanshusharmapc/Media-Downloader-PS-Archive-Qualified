"""Regression policy for MDPS-AUDIT2-125 saved-list upload date."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
src=(p.parse_args().source_root/"src/batchdownloader.cpp").read_text(encoding="utf-8")
parser=src[src.index("void batchdownloader::dataFromFile"):src.index("bool batchdownloader::showMetaData")]
assert 'dFileopts.uploadDate == "upload_date"' in parser
assert 'obj.value( "uploadDate" ).toString()' in parser
own=src[src.index("File created by us"):src.index("File created with yt-dlp")]
assert 'auto b = "upload_date"' in own
assert 'auto b = "uploadDate"' not in own
print("Saved-list upload date round-trip policy: PASS")
