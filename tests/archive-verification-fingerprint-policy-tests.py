"""Regression policy for MDPS-AUDIT2-017 media verification fingerprints."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/archive/archivecore.h").read_text(encoding="utf-8")
src=(root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")

for field in ("verifiedSha256","verifiedSize","verificationProfile"):
    assert field in hdr
assert '"verified_sha256"' in src and '"verified_size"' in src and '"verification_profile"' in src
assert 'mediaVerificationProfile="full-decode-v1"' in src
assert "verifiedRepresentationUnchanged" in src
sync=src[src.index("bool MediaExecutor::syncItem"):src.index("bool MediaExecutor::syncItems")]
assert "verifiedRepresentationUnchanged" in sync
assert sync.index("verifiedRepresentationUnchanged") < sync.index("check(representation.path)")
assert "duration*2000.0" in src
assert "24LL*60*60*1000" in src
assert "10*60*1000" in src
print("Media verification fingerprint policy: PASS")
