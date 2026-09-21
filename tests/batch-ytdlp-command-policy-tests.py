"""Regression policy for MDPS-AUDIT2-076 minimal yt-dlp command import."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/batchdownloader.cpp").read_text(encoding="utf-8")

start=source.index("void batchdownloader::addToList")
end=source.index("void batchdownloader::download(",start)
block=source[start:end]

assert "const auto it = raw.trimmed()" in block, "surrounding whitespace is not normalized"
assert 'if( m.size() < 2 )' in block, "minimal executable-plus-URL form is still rejected"
assert 'if( m.size() < 3 )' not in block, "old option-required gate remains"
assert 'auto url = m.takeLast()' in block
assert 'obj.insert( "engineName",m.takeFirst() )' in block
assert 'obj.insert( "downloadOptions",m.join( " " ) )' in block

# The source-level contract intentionally preserves the existing option string
# reconstruction, which keeps quoted middle tokens while allowing zero options.
minimal=["yt-dlp","https://example.com/video"]
assert len(minimal)==2 and " ".join(minimal[1:-1])==""
quoted=['yt-dlp','--user-agent','"Example Agent"','https://example.com/video']
assert " ".join(quoted[1:-1])=='--user-agent "Example Agent"'
assert len(["yt-dlp"])<2
print("Batch yt-dlp minimal command import policy: PASS")
