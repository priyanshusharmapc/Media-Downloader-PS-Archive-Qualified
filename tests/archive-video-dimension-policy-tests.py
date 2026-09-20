"""Regression policy for MDPS-AUDIT2-035 canonical video dimensions."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")

start=source.index("ValidationResult MediaVerifier::probe")
end=source.index("MediaExecutor::MediaExecutor",start)
body=source[start:end]

assert 'stream.value("width").toInt()>1920' in body
assert 'stream.value("height").toInt()>1080' in body
assert "1920x1080 canonical envelope" in body
assert "scale=w='min(1920,iw)':h='min(1080,ih)'" in source

print("Canonical video dimension contract policy: PASS")
