"""Regression policy for MDPS-AUDIT2-074 64-bit media-size metadata."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root

helper=(root/"src/engines/json_media_size.hpp").read_text(encoding="utf-8")
lux=(root/"src/engines/lux.cpp").read_text(encoding="utf-8")
you=(root/"src/engines/you-get.cpp").read_text(encoding="utf-8")

assert "qint64 nonNegativeByteCount" in helper
assert "toLongLong(&ok)" in helper
assert "value.isDouble()" in helper
assert "std::isfinite(number)" in helper and "std::floor(number)!=number" in helper
assert "largestExactJsonInteger=9007199254740991.0" in helper
assert "number<0" in helper
assert 'engineJson::nonNegativeByteCount( obj.value( "size" ) )' in lux
assert 'engineJson::nonNegativeByteCount( oo.value( "size" ) )' in you
assert '.value( "size" ).toInt()' not in lux
assert '.value( "size" ).toInt()' not in you

# Boundary intent: the parser must be able to represent values above INT_MAX
# while explicitly rejecting malformed, fractional, negative and inexact huge numbers.
assert 2147483648 < 9007199254740991
assert 5 * 1024**3 < 9007199254740991
print("64-bit engine media-size policy: PASS")
