"""Regression policy for MDPS-AUDIT2-143 multi-URL yt-dlp merge outputs."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
header=(root/"src/engines/yt-dlp.h").read_text(encoding="utf-8")
source=(root/"src/engines/yt-dlp.cpp").read_text(encoding="utf-8")
assert "m_mergeSeen" not in header
start=source.index("const QByteArray& yt_dlp::yt_dlplFilter::parseOutput")
end=source.index("void yt_dlp::yt_dlplFilter::setFileName",start)
body=source[start:end]
assert 'if( e.contains( " Merging formats into \\"" ) )' in body
assert "m_mergeSeen" not in body
assert "this->setFileName( m )" in body
setbody=source[end:source.index("\n}",end)+2]
assert "if( it == fn )" in setbody
print("yt-dlp multi-URL merge output policy: PASS")
