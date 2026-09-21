"""Regression for MDPS-AUDIT2-121 local-file open containment."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines.cpp").read_text(encoding="utf-8")
start=source.index("void engines::engine::baseEngine::openLocalFile")
end=source.index("engines::engine::baseEngine::onlineVersion",start)
body=source[start:end]

assert "QDir::isAbsolutePath( reported )" in body
assert "QDir::cleanPath( QDir( normalizedRoot ).filePath( reported ) )" in body
assert "canonicalFilePath()" in body
assert 'const auto rootPrefix = root.endsWith( \'/\' ) ? root : root + "/"' in body
assert "candidate.compare( root,caseSensitivity ) == 0" in body
assert "candidate.startsWith( rootPrefix,caseSensitivity )" in body
assert "if( !inRoot )" in body
assert "s.startsWith( ss )" not in body
print("open local file root-boundary policy: PASS")
