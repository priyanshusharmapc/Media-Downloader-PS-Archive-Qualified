"""Regression policy for AUDIT2-182 Retry Failed scope."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
cpp=(root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")
hdr=(root/"src/archive/archivetab.h").read_text(encoding="utf-8")
assert "operationRetryFailed(const archive::Source& source)" in hdr
retry=cpp[cpp.index("void ArchiveTab::retryFailed()"):cpp.index("void ArchiveTab::syncAll()")]
assert "operationRetryFailed(source)" in retry
assert "syncSelected();" not in retry
op=cpp[cpp.index("QString ArchiveTab::operationRetryFailed"):cpp.index("void ArchiveTab::runAsync",cpp.index("QString ArchiveTab::operationRetryFailed"))]
assert 'state=="failed"||state=="interrupted"' in op
assert "retryVideo=retryable(item.video.state)" in op
assert "retryAudio=retryable(item.audio.state)" in op
assert "executor.syncItem(item,retryVideo,retryAudio" in op
assert "QSet<QString> eligibleKeys" in op
assert "QSet<QString> processedKeys" in op
assert "processedKeys.contains(occurrence.itemKey)" in op
assert "PlaylistDiscovery" not in op
assert ".reconcile(" not in op
print("Archive Retry Failed scope policy: PASS")
