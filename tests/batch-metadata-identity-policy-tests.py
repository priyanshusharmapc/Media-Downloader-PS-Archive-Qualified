"""Regression policy for MDPS-AUDIT2-075 stable Batch metadata identity."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
cpp=(root/"src/batchdownloader.cpp").read_text(encoding="utf-8")
hdr=(root/"src/batchdownloader.h").read_text(encoding="utf-8")
util=(root/"src/utility.h").read_text(encoding="utf-8")

timer=cpp[cpp.index("void batchdownloader::getMetaData"):cpp.index("void batchdownloader::updateMetaData")]
assert "rowWithIdentity( identity )" in timer
assert "const auto identity = m_table.entryAt( row ).stableIdentity" in timer
done=cpp[cpp.index("void batchdownloader::showThumbnail"):cpp.index("int batchdownloader::addItemUi",cpp.index("void batchdownloader::showThumbnail"))]
assert "rowWithIdentity( m_identity )" in done
assert "m_parent.addItem( row" in done
network=cpp[cpp.index("void batchdownloader::networkData"):cpp.index("void batchdownloader::addItem(",cpp.index("void batchdownloader::networkData"))]
assert "rowWithIdentity( m.identity() )" in network
assert "m.index()" not in network
assert "const QString& identity() const" in hdr
assert "m_identity" in hdr
assert "stableIdentity" in cpp
assert "QString m_identity" in util
print("Stable Batch metadata callback identity policy: PASS")
