"""Regression policy for MDPS-AUDIT2-129 Batch media-part editor state."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/batchdownloader.cpp").read_text(encoding="utf-8")

open_start=source.index("void batchdownloader::setVisibleWidgetOverMainTable")
open_end=source.index("void batchdownloader::renameFile",open_start)
open_body=source[open_start:open_end]
assert "m_table.timeInterval( row )" in open_body
assert "m_table.chapters( row )" in open_body
assert "m_table.splitByChapters( row )" in open_body
assert "lineEditStartTimeInterval->clear()" in open_body
assert "lineEditEndTimeInterval->clear()" in open_body
assert "lineEditChapters->clear()" in open_body

set_start=source.index("void batchdownloader::setTimeIntervals")
set_end=source.index("void batchdownloader::showSubtitles",set_start)
set_body=source[set_start:set_end]
assert "if( a.isEmpty() != b.isEmpty() )" in set_body
assert "m_table.setTimeInterval( {},row )" in set_body
assert "m_table.setChapters( {},row )" in set_body
assert "removeUiOption" in set_body
assert "tableWidget::type::SplitByChapters" in set_body
print("Batch media-part editor synchronization policy: PASS")
