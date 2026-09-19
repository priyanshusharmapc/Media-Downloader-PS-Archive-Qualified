"""Regression for MDPS-AUDIT2-142 cancellation output ownership."""
from __future__ import annotations
import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
root = p.parse_args().source_root
cpp = (root / "src/engines/yt-dlp.cpp").read_text(encoding="utf-8")
header = (root / "src/engines/yt-dlp.h").read_text(encoding="utf-8")

assert "std::vector< QByteArray > m_ownedFileNames" in header
assert "utility::deleteTmpFiles( m,m_ownedFileNames )" in cpp
assert "utility::deleteTmpFiles( m,m_fileNames )" not in cpp

parse_start = cpp.index("const QByteArray& yt_dlp::yt_dlplFilter::parseOutput")
parse_end = cpp.index("void yt_dlp::yt_dlplFilter::setFileName", parse_start)
parse = cpp[parse_start:parse_end]
already = parse.index('has already been downloaded')
owned_false = parse.index("this->setFileName( m,false )", already)
destination = parse.index("] Destination: ", owned_false)
owned_default = parse.index("this->setFileName(", destination)
assert already < owned_false < destination < owned_default

set_start = cpp.index("void yt_dlp::yt_dlplFilter::setFileName")
set_body = cpp[set_start:]
assert "_add( m_fileNames,normalized )" in set_body
assert "if( ownedByInvocation )" in set_body
assert "_add( m_ownedFileNames,normalized )" in set_body
assert set_body.index("_add( m_fileNames,normalized )") < set_body.index("if( ownedByInvocation )") < set_body.index("_add( m_ownedFileNames,normalized )")

print("yt-dlp cancellation output ownership policy: PASS")
