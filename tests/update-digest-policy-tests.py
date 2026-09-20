"""Regression policy for MDPS-AUDIT2-203 GitHub release digest normalization."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
network=(p.parse_args().source_root/"src/networkAccess.cpp").read_text(encoding="utf-8")

helper=network[network.index("struct ParsedReleaseDigest"):network.index("QString restoreUpdateBackup")]
assert 'algorithm.compare( "sha256",Qt::CaseInsensitive )' in helper
assert "hex.size() != 64" in helper
assert "Malformed SHA-256 release digest" in helper
assert "out.sha256 = hex.toLower()" in helper

download=network[network.index("void networkAccess::uMediaDownloaderM"):network.index("void networkAccess::updateMediaDownloader( networkAccess::updateMDOptions")]
assert "const auto digest = parseReleaseSha256( md.hash )" in download
assert "if( !digest.valid )" in download
assert "if( !digest.present )" in download
assert "digest.sha256 == actual" in download
assert "md.hash == actual" not in download
assert "md.hash == m" not in download
assert "invalid release digest metadata" in download

print("GitHub release digest normalization policy: PASS")
