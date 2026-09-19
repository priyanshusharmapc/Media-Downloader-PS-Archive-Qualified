"""Package identity regression for MDPS-AUDIT2-004."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
cm=(root/"CMakeLists.txt").read_text(encoding="utf-8")
desktop=(root/"src/media-downloader-ps-archive.desktop").read_text(encoding="utf-8")
flat=(root/"src/flatpak/io.github.priyanshusharmapc.MediaDownloaderPSArchive.desktop").read_text(encoding="utf-8")
meta=(root/"src/flatpak/io.github.priyanshusharmapc.MediaDownloaderPSArchive.metainfo.xml").read_text(encoding="utf-8")

assert 'Media Downloader PS Archive' in cm
assert 'B6D5E5C4-3B5A-4C67-9A6C-92F27B7D4E31' in cm
assert 'https://github.com/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified' in cm
assert 'MyAppURL             \\"https://github.com/mhogomchungu/media-downloader\\"' not in cm
assert 'AppId={{6DD595EF-ECA2-481B-B008-CB7302603A0D' not in cm
assert 'OUTPUT_NAME media-downloader-ps-archive' in cm
assert 'OUTPUT_NAME io.github.priyanshusharmapc.MediaDownloaderPSArchive' in cm
assert 'Exec=media-downloader-ps-archive' in desktop
assert 'Icon=media-downloader-ps-archive' in desktop
assert 'Exec=io.github.priyanshusharmapc.MediaDownloaderPSArchive' in flat
assert '<id>io.github.priyanshusharmapc.MediaDownloaderPSArchive</id>' in meta
assert 'https://github.com/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified' in meta
assert 'mhogomchungu' not in meta
print("fork package identity policy: PASS")
