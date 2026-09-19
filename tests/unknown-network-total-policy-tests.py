"""Regression for MDPS-AUDIT2-095 unknown network totals."""
from pathlib import Path
import argparse

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/networkAccess.cpp").read_text(encoding="utf-8")
start=source.index("QString networkAccess::downloadSpeed::calculate")
body=source[start:]

assert "if( totalSize <= 0 )" in body
assert "if( totalSize == 0 )" not in body
unknown=body.index("if( totalSize <= 0 )")
percentage=body.index("double( received ) * 100 / double( totalSize )")
assert unknown < percentage
assert "std::min( 100.0,double( received ) * 100 / double( totalSize ) )" in body
print("unknown network total policy: PASS")
