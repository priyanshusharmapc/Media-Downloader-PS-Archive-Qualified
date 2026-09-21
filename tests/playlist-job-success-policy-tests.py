"""Regression policy for MDPS-AUDIT2-135 Playlist job-only all-success state."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
header=(root/"src/tableWidget.h").read_text(encoding="utf-8")
table=(root/"src/tableWidget.cpp").read_text(encoding="utf-8")
playlist=(root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")
assert "allFinishedWithSuccess( int firstRow = 0 )" in header
start=table.index("bool tableWidget::allFinishedWithSuccess")
end=table.index("int tableWidget::finishWithSuccess",start)
body=table[start:end]
assert "for( int i = firstRow" in body
assert "return m_table.rowCount() > firstRow" in body
assert "m_table.allFinishedWithSuccess( 1 )" in playlist
print("Playlist job-only all-success policy: PASS")
