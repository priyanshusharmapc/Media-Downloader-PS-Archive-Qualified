"""Regression policy for MDPS-AUDIT2-126 subtitle-kind propagation."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/batchdownloader.cpp").read_text(encoding="utf-8")

show=source.index("void batchdownloader::showSubtitles")
select=source.index("QString batchdownloader::setSubtitleString",show)
body=source[show:select]
assert 'obj.insert( "type",m )' in body
assert '_add( it,"automatic_captions" )' in body

end=source.index("void batchdownloader::showBDFrame",select)
sel=source[select:end]
assert 'obj.value( "type" ).toString()' in sel
assert 'type == "automatic_captions"' in sel
assert 'return "ac: " + m' in sel
assert 'return "su: " + m' in sel
print("Batch subtitle kind propagation policy: PASS")
