"""Regression policy for MDPS-AUDIT2-039 persisted-write integrity."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
header=(root/"src/networkAccess.h").read_text(encoding="utf-8")
source=(root/"src/networkAccess.cpp").read_text(encoding="utf-8")

assert "bool write( const QByteArray& e )" in header
assert "written != e.size()" in header
assert "m_writeFailed = true" in header
assert "m_file.get() == nullptr" in header
assert "bool close()" in header
assert "m_file->flush()" in header
assert "QFileDevice::NoError" in header
assert "bool verifyPersistedHash( const QByteArray& expected )" in header
assert "QCryptographicHash::Sha256" in header
assert "hash.addData( &persisted )" in header
assert "hash.result() != expected" in header
assert "bool writeFailed() const" in header

assert "if( md.file.write( data ) )" in source
assert "if( opts.file.write( data ) )" in source
assert source.index("if( md.file.write( data ) )") < source.index("md.hashCalculator->addData( data )")
assert source.index("if( opts.file.write( data ) )") < source.index("opts.hashCalculator->addData( data )")
assert "md.file.verifyPersistedHash( receivedHash )" in source
assert "opts.file.verifyPersistedHash( receivedHash )" in source
assert "receivedHash.toHex().toLower()" in source
print("Persisted write integrity policy: PASS")
