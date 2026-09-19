"""Regression policy for MDPS-AUDIT2-119 subscription corruption handling."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
header=(root/"src/playlistdownloader.h").read_text(encoding="utf-8")
source=(root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")

assert "bool load() ;" in header
assert "bool m_loaded = false ;" in header
assert "bool m_storeValid = true ;" in header

start=source.index("bool playlistdownloader::subscription::load()")
end=source.index("void playlistdownloader::banner::updateProgress",start)
body=source[start:end]
assert "QJsonDocument::fromJson" in body
assert "QJsonParseError::NoError" in body
assert "!doc.isArray()" in body
assert "m_storeValid = false" in body
assert "if( !m_storeValid )" in body
assert "return ;" in body
assert "editing is disabled" in body

for fn in [
    "void playlistdownloader::subscription::add",
    "void playlistdownloader::subscription::remove",
]:
    s=source.index(fn)
    e=source.index("\n}",s)+2
    section=source[s:e]
    assert "if( !this->load() )" in section
    assert "return ;" in section

print("Subscription corruption preservation policy: PASS")
