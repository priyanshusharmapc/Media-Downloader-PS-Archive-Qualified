"""Regression policy for MDPS-AUDIT2-018 bulk sync write amplification."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/archive/archivecore.h").read_text(encoding="utf-8")
src=(root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")

assert "beginRepresentationBatch" in hdr
assert "commitRepresentationBatch" in hdr
assert "m_representationBatchItems" in hdr
sync_item=src[src.index("bool MediaExecutor::syncItem"):src.index("bool MediaExecutor::syncItems")]
assert "rebuildProjections" in sync_item
assert "rebuildProjections&&!m_store.writeAllProjections" in sync_item
sync_items=src[src.index("bool MediaExecutor::syncItems"):src.index("RecoveryImporter::RecoveryImporter")]
assert "beginRepresentationBatch" in sync_items
assert "syncItem(item,true,true,&e,false)" in sync_items
assert sync_items.count("commitRepresentationBatch") == 1
assert sync_items.count("writeAllProjections") == 1
update=src[src.index("bool Store::updateRepresentation"):src.index("bool Store::updateCanonicalMetadata")]
assert "m_representationBatchActive" in update
assert "m_representationBatchItems=std::move(items)" in update
print("Bulk sync batching policy: PASS")

# A batch is canonical-item scoped. Duplicate input rows must not redownload or
# rewrite the same canonical item twice in one session.
assert "QSet<QString> processedKeys" in sync_items
assert "processedKeys.contains(item.key)" in sync_items
assert "processedKeys.insert(item.key)" in sync_items
