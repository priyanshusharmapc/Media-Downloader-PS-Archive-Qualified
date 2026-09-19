"""Regression policy for MDPS-AUDIT2-037 recovery-status revival."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")

assert "bool isSourceAccessible" in source
assert 'availability!="unknown"' in source
assert "void updateRecoveryStatus(CanonicalItem& item)" in source
assert 'item.recoveryStatus="unrecovered"' in source
assert 'item.recoveryStatus="not_required"' in source

rep0=source.index("bool Store::updateRepresentation")
rep1=source.index("bool Store::updateCanonicalMetadata",rep0)
assert "updateRecoveryStatus(item);" in source[rep0:rep1]

meta0=rep1
meta1=source.index("ReconcileSummary Store::reconcile",meta0)
assert "updateRecoveryStatus(item);" in source[meta0:meta1]

rec0=meta1
rec1=source.index("bool Store::writeProjections",rec0)
assert "updateRecoveryStatus(c);" in source[rec0:rec1]

assert 'canon.recoveryStatus=="unrecovered"' in source
print("Recovery-status revival policy: PASS")
