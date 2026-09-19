"""Regression policy for MDPS-AUDIT2-170 you-get filesystem output identity."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/engines/you-get.h").read_text(encoding="utf-8")
cpp=(root/"src/engines/you-get.cpp").read_text(encoding="utf-8")

assert "QByteArray m_outputFile" in hdr
start=cpp.index("const QByteArray& you_get::you_getFilter::operator()")
end=cpp.index("you_get::you_getFilter::~you_getFilter",start)
body=cpp[start:end]

assert "s.addFileName( m_outputFile )" in body
done=body[:body.index("}else if( s.lastLineIsProgressLine()")]
assert "s.addFileName( m_title )" not in done
assert 'captureOutputLine( "Merged into ",false )' in body
assert 'captureOutputLine( "Downloading ",true )' in body
assert "m.indexOf( '\\n',begin )" in body
assert "if( end == -1 )end = m.size()" in body
assert "m_outputFile = m.mid" in body or "m_outputFile = merged" in body
assert 'm_outputFile = m.mid( a + len' in body

print("you-get output filename identity policy: PASS")
