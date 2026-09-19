"""Regression policy for MDPS-AUDIT2-049 and MDPS-AUDIT2-065."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
utility=(root/"src/utility.cpp").read_text(encoding="utf-8")
batch=(root/"src/batchdownloader.cpp").read_text(encoding="utf-8")

us=utility[utility.index("void utility::saveDownloadList( const Context& ctx,tableWidget&"):utility.index("void utility::saveDownloadList( const Context& ctx,QMenu&")]
assert "QFile::remove( e )" not in us
assert "QSaveFile out( e )" in us
assert "!priorDoc.isArray()" in us
assert "out.write( m ) != m.size()" in us and "!out.commit()" in us

bs=batch[batch.index("void batchdownloader::getListFromFile( const QString& e,bool deleteFile )"):batch.index("void batchdownloader::getListFromFile( QMenu& m )")]
read_guard=bs.index("if( !readOk || list.isEmpty() )")
items_guard=bs.index("if( items.size() )")
remove=bs.index("QFile::remove( e )")
assert read_guard < items_guard < remove
assert "if( deleteFile && !QFile::remove( e ) )" in bs
print("Batch autosave durability policy: PASS")
