"""Regression policy for MDPS-AUDIT2-048 Basic Downloader empty input."""
from __future__ import annotations
import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
source = (p.parse_args().source_root / "src/basicdownloader.cpp").read_text(encoding="utf-8")
start = source.index("void basicdownloader::download( const QString& url )")
end = source.index("void basicdownloader::download( const basicdownloader::engine& engine", start)
body = source[start:end]

assert "url.trimmed().isEmpty()" in body
remove = body.index('m.removeAt( 0 )')
guard = body.index("if( m.isEmpty() )", remove)
last = body.index("m.last()", guard)
assert remove < guard < last
print("Basic Downloader empty-token guard policy: PASS")
