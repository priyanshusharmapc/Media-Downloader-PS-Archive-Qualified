"""Regression policy for MDPS-AUDIT2-073 gallery-dl incremental JSON framing."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/gallery-dl.cpp").read_text(encoding="utf-8")

start=source.index("bool gallery_dl::parse( const int& s")
end=source.index("std::vector< QByteArray > gallery_dl::parseJsonData",start)
body=source[start:end]

assert "for( int ss = s ; ss < data.size() ; ++ss )" in body, (
    "every byte access must be dominated by the current buffer bound")
assert "data.at( ss )" in body
assert "bool inString = false" in body and "bool escaped = false" in body
assert "m == '\\\\'" in body and 'm == \'"\'' in body, (
    "JSON string and escape state must prevent braces in strings changing depth")
assert "data.mid( s,ss - s + 1 )" in body, (
    "object extraction length must be relative to its opening offset")
assert "return true" in body.split("// Keep the incomplete object buffered until more bytes arrive.",1)[1]
assert "data[ ss ]" not in body
print("gallery-dl incremental JSON framing policy: PASS")
