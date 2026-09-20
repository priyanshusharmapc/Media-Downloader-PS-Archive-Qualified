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

# A syntactically valid JSON array is not enough: malformed records must not be
# silently coerced through QJsonValue::toString().
assert "!value.isObject()" in body
assert 'object.value( "uiName" ).isString()' in body
assert 'object.value( "url" ).isString()' in body
assert '!options.isUndefined() && !options.isString()' in body
assert "if( m_storeValid )m_array = array" in body

# Persistence must remain failure-atomic when editing a valid store.
assert "QSaveFile f( m_path )" in body
assert "f.cancelWriting()" in body
assert "return f.commit()" in body
assert "const auto previous = m_array" in body
assert "m_array = previous" in body
assert "this->setVisible( true )" in body
