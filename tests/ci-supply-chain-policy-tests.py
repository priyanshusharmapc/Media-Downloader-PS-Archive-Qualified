"""Regression policy for MDPS-AUDIT2-024 immutable qualification inputs."""
from __future__ import annotations
import argparse
import re
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
workflow=(p.parse_args().source_root/".github/workflows/archive-qt6.yml").read_text(encoding="utf-8")

assert "container: debian@sha256:" in workflow
assert "container: debian:trixie" not in workflow
uses=re.findall(r"uses:\s*([^\s#]+)",workflow)
assert uses
for ref in uses:
    assert re.search(r"@[0-9a-f]{40}$",ref), f"mutable action reference: {ref}"
assert "python3 -m compileall -q tests" in workflow
assert "python -m compileall -q tests" in workflow
assert "actions/upload-artifact@" not in workflow
assert "cache: false" in workflow
assert "artifact-gc:" in workflow and "actions: write" in workflow
assert "refs/heads/ci/" not in workflow
print("Immutable CI toolchain policy: PASS")
