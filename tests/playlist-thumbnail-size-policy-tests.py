"""Regression policy for MDPS-AUDIT2-115 Playlist thumbnail dimensions."""
from __future__ import annotations

import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
source = (p.parse_args().source_root / "src/playlistdownloader.cpp").read_text(encoding="utf-8")

start = source.index("void playlistdownloader::setThumbnail")
end = source.index("void playlistdownloader::reportFinishedStatus", start)
body = source[start:end]

assert "settings::tabName::playlist" in body
assert "settings::tabName::batch" not in body
assert "thumbnailWidth( a )" in body
assert "thumbnailHeight( a )" in body

print("Playlist GalleryDL thumbnail sizing policy: PASS")

network_start=source.index("void playlistdownloader::networkData")
network_end=source.index("void playlistdownloader::addTextToUi",network_start)
network=source[network_start:network_end]
assert "settings::tabName::playlist" in network
assert "settings::tabName::batch" not in network
assert "thumbnailWidth( settings::tabName::playlist )" in network
assert "thumbnailHeight( settings::tabName::playlist )" in network
