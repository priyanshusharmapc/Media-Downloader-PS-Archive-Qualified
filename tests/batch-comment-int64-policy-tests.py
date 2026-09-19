"""Regression policy for MDPS-AUDIT2-103 64-bit comment metadata."""
from __future__ import annotations

import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
source = (p.parse_args().source_root / "src/batchdownloader.cpp").read_text(encoding="utf-8")

assert "bool _commentInt64" in source
assert "std::isfinite" in source
assert "std::floor( number ) != number" in source
assert "std::numeric_limits< qint64 >::min()" in source
assert "std::numeric_limits< qint64 >::max()" in source

start = source.index("void _add_comments")
end = source.index("void batchdownloader::saveComments", start)
body = source[start:end]
assert 'obj.value( "like_count" ).toInt()' not in body
assert 'timestamp.toInt()' not in body
assert "_commentInt64( obj.value( "like_count" ),likeCountValue )" in body
assert "_commentInt64( timestamp,timestampValue )" in body
assert "utility::fromSecsSinceEpoch( timestampValue )" in body

sort_start = source.index("auto _make_sort")
sort_end = source.index("bool batchdownloader::saveSubtitles", sort_start)
sort_body = source[sort_start:sort_end]
assert "operator qint64() const" in sort_body
assert "std::less<qint64>()" in sort_body
assert "std::greater<qint64>()" in sort_body
assert ".toInt()" not in sort_body

print("64-bit comment metadata policy: PASS")
