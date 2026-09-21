"""Regression policy for MDPS-AUDIT2-138 per-job destination lifetime."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root

table=(root/"src/tableWidget.h").read_text(encoding="utf-8")
utility=(root/"src/utility.h").read_text(encoding="utf-8")
engines=(root/"src/engines.cpp").read_text(encoding="utf-8")
batch_h=(root/"src/batchdownloader.h").read_text(encoding="utf-8")
batch=(root/"src/batchdownloader.cpp").read_text(encoding="utf-8")
playlist=(root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")
basic=(root/"src/basicdownloader.cpp").read_text(encoding="utf-8")
lux_h=(root/"src/engines/lux.h").read_text(encoding="utf-8")
lux=(root/"src/engines/lux.cpp").read_text(encoding="utf-8")

assert "QString downloadFolder ;" in table
assert "void setDownloadFolder" in table
assert "table.entryAt( index ).downloadFolder" in utility
assert "runCommandOnDownloadedFile( fileNames,downloadFolder )" in utility

open_body=engines[engines.index("void engines::openUrls"):engines.index("const QString& engines::defaultEngineName")]
assert "entry.downloadFolder" in open_body

assert "m_parent.m_table.setDownloadFolder( m_index,m_downloadFolder )" in batch_h
assert "return m_downloadFolder" in batch_h
rename=batch[batch.index("void batchdownloader::renameFile"):batch.index("void batchdownloader::setTimeIntervals")]
assert "entry.downloadFolder" in rename
batch_thumb=batch[batch.index("void batchdownloader::setThumbnail"):batch.index("void batchdownloader::updateMetaData")]
assert "entry.downloadFolder" in batch_thumb

pl_start=playlist.index("void playlistdownloader::downloadRecursively")
pl_end=playlist.index("void playlistdownloader::getListing",pl_start)
pl_job=playlist[pl_start:pl_end]
assert "m_parent.m_table.setDownloadFolder( m_index,m_downloadFolder )" in pl_job
assert "return m_downloadFolder" in pl_job
pl_thumb=playlist[playlist.index("void playlistdownloader::setThumbnail"):playlist.index("void playlistdownloader::reportFinishedStatus")]
assert "entry.downloadFolder" in pl_thumb

basic_events=basic[basic.index("class events"):basic.index("events ev")]
assert "m_downloadFolder( p.m_settings.downloadFolder() )" in basic_events
assert "setDownloadFolder( 0,m_downloadFolder )" in basic_events
assert "return m_downloadFolder" in basic_events

assert "QString m_downloadFolder ;" not in lux_h
assert "m_downloadFolder( engines.Settings().downloadFolder()" not in lux
assert 'const auto folder = this->downloadFolder( this->Settings().downloadFolder() ) + "/"' in lux

print("Per-job destination lifetime policy: PASS")
