"""Regression for MDPS-AUDIT2-093 duration formatting beyond 24h."""
from pathlib import Path
import argparse

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/logger.h").read_text(encoding="utf-8")
start=source.index("static QString secondsToString")
end=source.index("private:",start)
body=source[start:end]

assert "QTime" not in body
assert "const auto hours = total / 3600" in body
assert "const auto minutes = totalMinutes % 60" in body
assert "const auto seconds = total % 60" in body
assert 'QString( "%1:%2:%3" )' in body
assert "s > 0 ? s : 0" in body
print("long duration formatting policy: PASS")
