"""Regression policy for MDPS-AUDIT2-168 Wget filename quoting."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/engines/wget.cpp").read_text(encoding="utf-8")
assert "static QByteArray wgetReportedFileName" in s
assert 'value.startsWith( "'" ) && value.endsWith( "'" )' in s
assert 'value.startsWith( left ) && value.endsWith( right )' in s
assert "m_title = wgetReportedFileName( it.mid( 11 ) )" in s
assert "auto s = wgetReportedFileName( it.mid( 7 ) )" in s
start=s.index("const QByteArray& wget::wgetFilter::processWget1")
assert '.replace( "'", "" )' not in s[start:]
print("Wget outer-quote filename policy: PASS")
