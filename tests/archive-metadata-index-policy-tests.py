"""Regression policy for MDPS-AUDIT2-026 canonical metadata linkage."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/archive/archivecore.h").read_text(encoding="utf-8")
cpp=(root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")
assert "const QString& metadataPath = {}" in hdr
update=cpp[cpp.index("bool Store::updateRepresentation"):cpp.index("bool Store::updateCanonicalMetadata")]
assert 'metadataPath.startsWith("Metadata/")' in update
assert "m_paths.isSafeRelative(metadataPath)" in update
assert "detail::noLinks(absoluteMetadata)" in update
assert "item.metadataPath=metadataPath" in update
video=cpp[cpp.index("bool MediaExecutor::downloadVideo"):cpp.index("bool MediaExecutor::downloadAudio")]
assert "QTemporaryDir metadataStaging" in video
assert 'metadataPathCandidate="Metadata/youtube-"+item.providerId+"-"' in video
assert "validateOwnedStagingTree" in video
assert "metadataPath=metadataPathCandidate" in video
assert 'updateRepresentation(item.key,"video",done,&finalizationError,metadataPath)' in video
safety=(root/"src/archive/archivesafety.h").read_text(encoding="utf-8")
assert 'metadataPath.startsWith("Metadata/")' in safety
assert '"Invalid canonical metadata path"' in safety
assert 'Canonical item metadata directory is missing or linked' in cpp
print("Canonical metadata linkage policy: PASS")
