"""Regression policy for MDPS-AUDIT2-159 context-menu engine target rows."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
for name in ["batchdownloader.cpp","playlistdownloader.cpp"]:
    source=(root/"src"/name).read_text(encoding="utf-8")
    anchor='auto mm = m.addMenu( utility::stringConstants::engineName().replace( ":","" ) )'
    start=source.index(anchor)
    end=source.index("auto subMenu",start)
    body=source[start:end]
    assert "engineTargetUrl" in body
    assert "engineTargetText" in body
    assert "[ this,row,engineTargetUrl,engineTargetText ]" in body
    assert "row < 0 || row >= m_table.rowCount()" in body
    assert "m_table.url( row ) != engineTargetUrl" in body
    assert "m_table.entryAt( row ).uiText != engineTargetText" in body
    assert "setDownloadingOptions( u,row,ac->objectName() )" in body
    assert "m_table.currentRow()" not in body
print("Context-menu engine target-row policy: PASS")
