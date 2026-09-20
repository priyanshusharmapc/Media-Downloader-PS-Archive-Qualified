"""Regression policy for MDPS-AUDIT2-057 Playlist remove control."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
src=(p.parse_args().source_root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")
start=src.index('ac = m.addAction( tr( "Remove" ) )')
end=src.index('if( m_settings.autoHideDownloadWhenCompleted() )',start)
body=src[start:end]
assert "m_table.removeAllSelected()" in body
assert "pbPLDownload->setEnabled" in body
assert "pbBDDownload" not in body
print("Playlist remove control policy: PASS")
