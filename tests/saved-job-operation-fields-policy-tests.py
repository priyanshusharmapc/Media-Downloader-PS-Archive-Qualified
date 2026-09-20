"""Regression policy for MDPS-AUDIT2-128 saved job operation fields."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
utility=(root/"src/utility.cpp").read_text(encoding="utf-8")
header=(root/"src/batchdownloader.h").read_text(encoding="utf-8")
batch=(root/"src/batchdownloader.cpp").read_text(encoding="utf-8")

save=utility[utility.index("static QJsonArray _saveDownloadList"):utility.index("void utility::saveDownloadList",utility.index("static QJsonArray _saveDownloadList"))]
for key in ["subtitle","timeInterval","chapters","splitByChapters","savedJobSchemaVersion"]:
    assert f'obj.insert( "{key}"' in save

for expr in [
    'subtitle( obj.value( "subtitle" ).toString() )',
    'timeInterval( obj.value( "timeInterval" ).toString() )',
    'chapters( obj.value( "chapters" ).toString() )',
    'splitByChapters( obj.value( "splitByChapters" ).toBool() )',
]:
    assert expr in header

start=batch.index("void batchdownloader::addItemToUi")
end=batch.index("void batchdownloader::addItemUiSlot",start)
body=batch[start:end]
assert "tableWidget::type::subtitleOption" in body
assert "tableWidget::type::DownloadTimeInterval" in body
assert "tableWidget::type::DownloadChapters" in body
assert "tableWidget::type::SplitByChapters" in body
print("Saved job operation-state round-trip policy: PASS")
