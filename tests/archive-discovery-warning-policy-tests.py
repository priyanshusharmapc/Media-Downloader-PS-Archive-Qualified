"""Regression policy for MDPS-AUDIT2-044 benign discovery warnings."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")
start=source.index("Snapshot PlaylistDiscovery::parse")
end=source.index("MediaVerifier::MediaVerifier",start)
body=source[start:end]

assert 'QRegularExpression("(?im)^\\\\s*ERROR:")' in body
assert '(?:ERROR|WARNING)' not in body
assert "s.complete=!transient && !malformed && !truncated && !reportedError && exitCode==0" in body
assert "playlist_count" in body and "n_entries" in body
assert "source.key" in body and 'root.value("id")' in body

print("Benign discovery warning policy: PASS")
