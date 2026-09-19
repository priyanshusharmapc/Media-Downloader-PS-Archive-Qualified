"""Regression policy for MDPS-AUDIT2-026 canonical metadata linkage."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/archive/archivecore.h").read_text(encoding="utf-8")
cpp=(root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")

assert "const QString& metadataPath = {}" in hdr
update=cpp[cpp.index("bool Store::updateRepresentation"):cpp.index("bool Store::updateCanonicalMetadata")]
assert 'metadataPath.startsWith("Metadata/")' in update
assert "m_paths.isSafeRelative(metadataPath)" in update
assert "detail::noLinks(absoluteMetadata)" in update
assert "item.metadataPath=metadataPath" in update
assert update.index("item.metadataPath=metadataPath") < update.index("return saveCanonicalItems(items,error)")

video=cpp[cpp.index("bool MediaExecutor::downloadVideo"):cpp.index("bool MediaExecutor::downloadAudio")]
assert 'info.fileName().contains("["+item.providerId+"]")' in video
assert 'metadataPath="Metadata/"+info.fileName()' in video
assert 'updateRepresentation(item.key,"video",done,error,metadataPath)' in video

print("Canonical metadata linkage policy: PASS")
