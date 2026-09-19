"""Regression policy for MDPS-AUDIT2-029 duplicate occurrence continuity."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")
start=source.index("ReconcileSummary Store::reconcile")
end=source.index("bool Store::writeProjections",start)
body=source[start:end]

assert "priorResolvedOccurrences" in body
assert "nextOccurrence=priorOccurrences" in body
assert "positionMatches.size()==1" in body
assert "positionMatches.isEmpty()&&candidates.size()==1" in body
assert "p.entryKey=prior[occurrencePrior].entryKey" in body
assert "matchedPrior.insert(occurrencePrior)" in body
assert 'p.entryKey=p.itemKey+"#"+QString::number(++nextOccurrence[p.itemKey])' in body
assert "observedOccurrences[p.itemKey]" not in body

print("Duplicate occurrence conservative matching policy: PASS")
