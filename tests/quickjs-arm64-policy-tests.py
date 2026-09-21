"""Regression policy for MDPS-AUDIT2-176 QuickJS ARM64 selection."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/quickjs.cpp").read_text(encoding="utf-8")

start=source.index("QString quickjs::namePrefix()")
end=source.index("QString quickjs::urlFileName",start)
selector=source[start:end]
assert "cpu.aarch64()" in selector
assert "return {} ;" in selector
assert '"-x86_64"' in selector

url=source[source.index("QString quickjs::urlFileName"):source.index("engines::metadata quickjs::parseJsonDataFromGitHub")]
assert "prefix.isEmpty() ? QString()" in url

meta=source[source.index("engines::metadata quickjs::parseJsonDataFromGitHub"):source.index("engines::engine::baseEngine::removeFilesStatus",source.index("engines::metadata quickjs::parseJsonDataFromGitHub"))]
assert "!prefix.isEmpty()" in meta

found=source[source.index("bool quickjs::foundNetworkUrl"):source.index("QString quickjs::parseVersionInfo",source.index("bool quickjs::foundNetworkUrl"))]
assert "!prefix.isEmpty()" in found
print("QuickJS ARM64 selection policy: PASS")
