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
assert 'const auto isAutosaveSnapshot = deleteFile && e.contains( ".consume-" )' in bs
assert "if( !readOk || list.isEmpty() )" in bs
assert "discardSnapshot()" in bs
assert "if( items.size() )" in bs
assert 'QLockFile lock( shared + ".lock" )' in bs
assert "if( currentBytes == list )" in bs
assert "QFile::exists( shared ) && !QFile::remove( shared )" in bs
assert "else if( deleteFile && !QFile::remove( e ) )" in bs
assert bs.index("if( items.size() )") < bs.index("if( currentBytes == list )")
print("Batch autosave durability policy: PASS")
