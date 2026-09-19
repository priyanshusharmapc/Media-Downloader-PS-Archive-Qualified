"""Regression policy for MDPS-AUDIT2-119 subscription corruption preservation."""
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
assert "!document.isArray()" in body
assert "m_storeValid = false" in body
assert "if( !this->load() )" in body
assert "if( !m_storeValid )" in body
assert "QSaveFile f( m_path )" in body
assert "m_array = previous" in body
assert "this->setVisible( true )" in body
assert "editing is disabled" in body
print("subscription corruption + atomic preservation policy: PASS")
