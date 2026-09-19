"""Regression policy for MDPS-AUDIT2-028 endurance provenance."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"scripts/archive-endurance.ps1").read_text(encoding="utf-8")

assert "[Parameter(Mandatory=$true)][string]$ExpectedCommit" in source
assert "[Parameter(Mandatory=$true)][string]$ExpectedCiRun" in source
assert "[string[]]$AllowedOverlayPath" in source
assert "function Assert-ExecutionPackage" in source
assert "Execution package differs from sealed base outside declared overlay" in source
assert "archiveCliSha256" in source
assert "$executionSeal=Assert-ExecutionPackage" in source
assert "executionSeal=$executionSeal" in source
print("Endurance execution provenance policy: PASS")
