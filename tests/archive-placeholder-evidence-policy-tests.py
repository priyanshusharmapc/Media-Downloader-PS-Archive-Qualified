"""Regression policy for MDPS-AUDIT2-051 discovery placeholder evidence."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")
start=source.index("PlaylistDiscovery::parse")
end=source.index("MediaVerifier::MediaVerifier",start)
body=source[start:end]

assert "observedAvailability=availabilityFromEntry(e)" in body
assert "p.providerId.isEmpty()&&rawUrl.isEmpty()&&!isUnavailable(observedAvailability)" in body
assert "malformed=true;" in body
assert 'p.title="[Unavailable item]"' in body
assert "p.availability=observedAvailability" in body

guard=body.index("p.providerId.isEmpty()&&rawUrl.isEmpty()&&!isUnavailable(observedAvailability)")
synth=body.index('p.title="[Unavailable item]"')
assert guard < synth
print("Discovery placeholder minimum-evidence policy: PASS")

assert "p.availability=observedAvailability" in body
