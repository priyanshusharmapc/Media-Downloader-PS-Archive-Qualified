"""Regression policy for MDPS-AUDIT2-067 comments export naming."""
from __future__ import annotations
import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
root = p.parse_args().source_root
source = (root / "src/batchdownloader.cpp").read_text(encoding="utf-8")

assert 'MediaDownloaderComments.json' in source
assert 'MediaDowloaderComments.json' not in source
print("Batch comments fallback filename policy: PASS")
