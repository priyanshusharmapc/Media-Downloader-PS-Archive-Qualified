"""Regression policy for MDPS-AUDIT2-134 subscription auto-download materialization."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
header=(root/"src/playlistdownloader.h").read_text(encoding="utf-8")
source=(root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")

assert "m_pendingRowMaterializations = 0" in header
assert "m_autoDownloadAfterMaterialization = false" in header

listing=source[source.index("void playlistdownloader::getListing"):source.index("void playlistdownloader::getList( playlistdownloader::listIterator",source.index("void playlistdownloader::getListing"))]
assert "m_pendingRowMaterializations = 0" in listing
assert "m_autoDownloadAfterMaterialization = false" in listing

parse=source[source.index("bool playlistdownloader::parseJson"):source.index("void playlistdownloader::networkResult",source.index("bool playlistdownloader::parseJson"))]
assert "m_pendingRowMaterializations++" in parse
assert parse.index("m_pendingRowMaterializations++") < parse.index("if( !thumbnailUrl.isEmpty() )")

net=source[source.index("void playlistdownloader::networkData"):source.index("void playlistdownloader::addTextToUi",source.index("void playlistdownloader::networkData"))]
assert "m_pendingRowMaterializations--" in net
assert "m_pendingRowMaterializations == 0 && m_autoDownloadAfterMaterialization" in net
assert "this->download()" in net

assert "m_parent.m_pendingRowMaterializations == 0" in source
assert "m_parent.m_autoDownloadAfterMaterialization = true" in source
print("Playlist row materialization barrier policy: PASS")
