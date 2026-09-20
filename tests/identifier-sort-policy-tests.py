"""Regression policy for MDPS-AUDIT2-141 identifier sorting."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/tableWidget.h").read_text(encoding="utf-8")
start=s.index("void arrangeTable( bool ascending,int column )")
end=s.index("template< typename Rows >",start)
body=s[start:end]
assert "a.toLongLong( &aNumber )" in body
assert "b.toLongLong( &bNumber )" in body
assert "if( an != bn )" in body
assert "QString::compare( a,b,Qt::CaseSensitive )" in body
assert "if( aNumber && bNumber )" in body
assert "QString::compare( a,b,Qt::CaseInsensitive )" in body
assert "tableWidget::compare( a,b,m_ascending )" not in body.split("}else{",1)[0]
print("Identifier sort semantics policy: PASS")
