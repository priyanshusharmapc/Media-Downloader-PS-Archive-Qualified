"""Regression policy for MDPS-AUDIT2-050 and MDPS-AUDIT2-064."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/batchdownloader.cpp").read_text(encoding="utf-8")
start=source.index("void batchdownloader::batchDownloaderSet()")
end=source.index("void batchdownloader::updateTitleBar()",start)
body=source[start:end]
subtitle=body[body.index("if( m_listType == batchdownloader::listType::SUBTITLES )"):]

row=subtitle.index("auto row = m_tableWidgetBDList.currentRow()")
guard=subtitle.index("row < 0")
item=subtitle.index("m_tableWidgetBDList.item( row,0 )")
stuff=subtitle.index("m_tableWidgetBDList.stuffAt( row )")
assert row < guard < item < stuff
assert "row >= m_tableWidgetBDList.rowCount()" in subtitle[guard:item]
assert "crow < 0" in subtitle[guard:item]
assert "crow >= m_table.rowCount()" in subtitle[guard:item]
print("subtitle selection bounds policy: PASS")
