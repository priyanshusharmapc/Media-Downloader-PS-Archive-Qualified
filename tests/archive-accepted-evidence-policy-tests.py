"""Regression policy for MDPS-AUDIT2-014 Accepted evidence integrity."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")

assert "bool verifyAcceptedEvidence" in source
assert 'receipt.value("file_sha256").isObject()' in source
assert "QDirIterator entries(packageDir" in source
assert "Unrecorded file in Accepted recovery package" in source
assert "Accepted recovery evidence hash mismatch" in source
assert "Accepted recovery evidence file is missing" in source
init=source[source.index("bool Store::initialize"):source.index("QVector<Source> Store::loadSources")]
assert "verifyAcceptedEvidence(m_paths,error)" in init
assert init.index("verifyAcceptedEvidence(m_paths,error)") < init.index("recoverStaleRunning")
print("Accepted recovery evidence integrity policy: PASS")

# Post-acceptance integrity must inspect directory/link/special entries too, not
# only files recorded in the receipt.
verify=source[source.index("bool verifyAcceptedEvidence"):source.index("bool Store::initialize")]
assert "QDir::AllEntries" in verify and "QDir::System" in verify
assert "info.isSymLink()" in verify
assert "if(info.isDir())continue" in verify
assert "if(!info.isFile())" in verify
assert "QDir::Files|QDir::Hidden" not in verify
