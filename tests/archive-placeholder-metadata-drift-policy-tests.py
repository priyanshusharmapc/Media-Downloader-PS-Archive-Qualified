"""Source-policy companion for the unresolved placeholder metadata-drift collision."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
src=(root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")
test=(root/"tests/archive-hardening-tests.cpp").read_text(encoding="utf-8")
assert "conservativePlaceholderDriftCandidate" in src
assert "stableUnavailableClass" in src
assert "stableUrl||stablePosition||stableNeighbors" in src
assert "candidates.size()==1?candidates.front():-1" in src
assert "priorIndexForPlaceholder=conservativePlaceholderDriftCandidate(p,snapshotIndex)" in src
assert 'name=="placeholder-metadata-drift"' in test
assert "metadata drift split canonical placeholder identity" in test
assert "ambiguous drift guessed an old occurrence" in test
assert "unrelated replacement was merged into old placeholder" in test
print("placeholder metadata-drift policy: PASS")
