"""Regression for MDPS-AUDIT2-100 stable Batch chooser target."""
from pathlib import Path
import argparse

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
cpp=(root/"src/batchdownloader.cpp").read_text(encoding="utf-8")
header=(root/"src/batchdownloader.h").read_text(encoding="utf-8")

assert "int m_listTargetRow = -1" in header
assert "QString m_listTargetUrl" in header

show=cpp[cpp.index("void batchdownloader::showList"):cpp.index("void batchdownloader::setDownloadingOptions")]
assert "m_listTargetRow = row" in show
assert "m_listTargetUrl = row >= 0" in show
assert "m_listTargetRow = -1" in show

double=cpp[cpp.index("void batchdownloader::tableItemDoubleClicked"):cpp.index("void batchdownloader::batchDownloaderSet")]
assert "const auto crow = m_listTargetRow" in double
assert "m_table.currentRow()" not in double
assert "m_table.url( crow ) != m_listTargetUrl" in double

setter=cpp[cpp.index("void batchdownloader::batchDownloaderSet"):cpp.index("void batchdownloader::updateTitleBar")]
assert "const auto crow = m_listTargetRow" in setter
assert "m_table.currentRow()" not in setter
assert "m_table.url( crow ) != m_listTargetUrl" in setter
assert "row < 0 || row >= m_tableWidgetBDList.rowCount()" in setter

assert "m_parent.m_table.url( m_row ) != m_url" in show
assert "m_parent.m_table.replace( array,m_row )" in show
assert show.index("m_parent.m_table.url( m_row ) != m_url") < show.index("m_parent.m_table.replace( array,m_row )")
assert "events ev( *this,listType,engine,row,url )" in show
print("batch chooser target policy: PASS")
