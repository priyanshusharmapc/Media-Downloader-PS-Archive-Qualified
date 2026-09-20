"""Regression for MDPS-AUDIT2-094 history-size bounds."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/settings.cpp").read_text(encoding="utf-8")

start=source.index("void settings::addToHistory")
end=source.index("void settings::addToplaylistRangeHistory",start)
body=source[start:end]
assert "if( max <= 0 )" in body
assert "history.clear()" in body
assert "while( history.size() > max )" in body
assert "input.trimmed().isEmpty() || history.contains( input )" in body
assert "while( history.size() >= max )" in body
assert "history.size() == max" not in body

hstart=source.index("int settings::historySize()")
hend=source.index("QString settings::thumbnailTabName",hstart)
hbody=source[hstart:hend]
assert "size > 0 ? size : 0" in hbody
print("history bounds policy: PASS")
