"""Regression policy for MDPS-AUDIT2-175 custom plugin install rollback."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines.cpp").read_text(encoding="utf-8")
start=source.index("QString engines::addEngine( const QByteArray& data")
end=source.index("void engines::removeEngine",start)
body=source[start:end]

assert "QFileInfo::exists( path )" in body
assert "Plugin definition already exists" in body
assert "QSaveFile file( path )" in body
assert "file.write( data ) != data.size()" in body
assert "!file.commit()" in body
assert "if( this->addEngine( extensionFileName,id ) )" in body
assert "utility::removeFile( path )" in body
assert body.index("utility::removeFile( path )") > body.index("if( this->addEngine( extensionFileName,id ) )")
assert "QIODevice::WriteOnly | QIODevice::Truncate" not in body

print("Custom plugin install rollback policy: PASS")
