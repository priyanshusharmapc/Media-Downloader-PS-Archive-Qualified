"""Regression policy for MDPS-AUDIT2-169 aria2c multi-URL completion."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
aria=(root/"src/engines/aria2c.cpp").read_text(encoding="utf-8")
logger=(root/"src/logger.h").read_text(encoding="utf-8")

start=aria.index("const QByteArray& aria2c::aria2c_dlFilter::operator()")
end=aria.index("aria2c::aria2c_dlFilter::~aria2c_dlFilter",start)
body=aria[start:end]
segment=body[body.index('if( e.contains( " Download complete: " )'):body.index('else if( e.contains( "Unrecognized URI',body.index('if( e.contains( " Download complete: " )'))]
assert "s.addFileName( fileName )" in segment
assert "continue ;" in segment
assert "break ;" not in segment
assert ".trimmed()" in segment

# Repeated scans are safe because the shared filename registry de-duplicates.
lstart=logger.index("void addFileName( const QByteArray& e )")
lend=logger.index("const std::vector< QByteArray >& fileNames()",lstart)
lbody=logger[lstart:lend]
assert "if( it == e )" in lbody
assert "return ;" in lbody
assert "m_fileNames.emplace_back( e )" in lbody

print("aria2c multi-completion policy: PASS")
