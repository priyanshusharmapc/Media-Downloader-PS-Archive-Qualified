"""Regression policy for MDPS-AUDIT2-130 recursive engine fallback isolation."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
batch=(root/"src/batchdownloader.cpp").read_text(encoding="utf-8")
playlist=(root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")

bs=batch[batch.index("void batchdownloader::downloadRecursively"):batch.index("void batchdownloader::addTextToUi",batch.index("void batchdownloader::downloadRecursively"))]
assert "m_defaultEngine( defaultEngine )" in bs
assert "m_parent.downloadRecursively( m_defaultEngine,m )" in bs
assert "meaw( *this,eng,engine,index )" in bs
assert "m_parent.downloadRecursively( m_engine,m )" not in bs

ps=playlist[playlist.index("void playlistdownloader::downloadRecursively"):playlist.index("void playlistdownloader::showBanner",playlist.index("void playlistdownloader::downloadRecursively"))]
assert "m_defaultEngine( defaultEngine )" in ps
assert "m_parent.downloadRecursively( m_defaultEngine,m,m_downloadRecursively )" in ps
assert "events( *this,eng,engine,index,downloadRecursively )" in ps
assert "m_parent.downloadRecursively( m_engine,m,m_downloadRecursively )" not in ps
print("Recursive engine fallback isolation policy: PASS")
