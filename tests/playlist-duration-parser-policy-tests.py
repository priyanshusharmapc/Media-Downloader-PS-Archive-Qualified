"""Regression for MDPS-AUDIT2-112 playlist media-length parsing."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines.cpp").read_text(encoding="utf-8")
start=source.index("int engines::engine::baseEngine::timer::toSeconds")
end=source.index("qint64 engines::engine::baseEngine::timer::elapsedTime()",start)
body=source[start:end]

assert "util::split( e,':',false )" in body
assert "return 60 * minutes + seconds" in body
assert "return 3600 * hours + 60 * minutes + seconds" in body
assert "minutes >= 60 || seconds >= 60" in body
assert "seconds >= 60" in body
assert "number < 0" in body
assert "3600 * _toNumber( m[ 0 ] ) + 360 * _toNumber( m[ 1 ] )" not in body
print("playlist duration parser policy: PASS")
