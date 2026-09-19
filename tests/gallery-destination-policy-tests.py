"""Regression policy for MDPS-AUDIT2-173 gallery-dl effective destination."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/gallery-dl.cpp").read_text(encoding="utf-8")

start=source.index("void gallery_dl::updateDownLoadCmdOptions")
end=source.index("QString gallery_dl::updateTextOnCompleteDownlod",start)
build=source[start:end]
assert "selectedOptions = opts.uiOptions.isEmpty() ? opts.userOptions : opts.uiOptions" in build
assert "selectedOptions = opts.userOptions.isEmpty() ? opts.uiOptions : opts.userOptions" in build
assert "!hasDestination( opts.ourOptions )" in build
assert "!hasDestination( selectedOptions )" in build
assert "!hasDestination( extraOpts )" in build
assert 'opts.ourOptions.prepend( "-d" )' in build

fstart=source.index("const QByteArray& gallery_dl::gallery_dlFilter::operator()")
fend=source.index("gallery_dl::gallery_dlFilter::~gallery_dlFilter",fstart)
body=source[fstart:fend]
segment=body[body.index("for( int i = 0 ; i + 1 < m.size()"):body.index("}",body.index("for( int i = 0 ; i + 1 < m.size()"))+2]
assert 'm[ i ] == "-d" || m[ i ] == "-D"' in body
assert "m_dir = QDir::fromNativeSeparators( m[ i + 1 ] ).toUtf8()" in body
# The destination scan must not stop at the first option.
scan=body[body.index("for( int i = 0 ; i + 1 < m.size()"):body.index("\n\t\t}",body.index("for( int i = 0 ; i + 1 < m.size()")))]
assert "break" not in scan

print("gallery-dl effective destination policy: PASS")
