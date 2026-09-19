"""Regression policy for MDPS-AUDIT2-149/153 Configure draft ownership."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/configure.h").read_text(encoding="utf-8")
cpp=(root/"src/configure.cpp").read_text(encoding="utf-8")

for member in ["m_textEncodingEngine","m_editOptionEngine","m_editOptionOldValue"]:
    assert member in hdr

pop=cpp[cpp.index("void configure::populateOptionsTable"):cpp.index("void configure::tabExited")]
assert "sameEncodingEngine" in pop
assert "m_settings.setTextEncoding( m_ui.lineEditConfigureTextEncoding->text(),m_textEncodingEngine )" in pop
assert "if( !sameEncodingEngine )" in pop
assert "m_textEncodingEngine = engineName" in pop

save=cpp[cpp.index("void configure::saveOptions"):cpp.index("void configure::setEngineOptions")]
assert "encodingEngine = m_textEncodingEngine.isEmpty() ? mm : m_textEncodingEngine" in save

assert "m_editOptionEngine = m_ui.cbConfigureEngines->currentText()" in cpp
assert "m_editOptionOldValue = m_tableDefaultDownloadOptions.item( row,1 ).text()" in cpp
assert "m_downloadEngineDefaultOptions.replace( m_editOptionEngine,m_editOptionOldValue,New )" in cpp
assert "m_ui.cbConfigureEngines->currentText() == m_editOptionEngine" in cpp

print("Configure per-engine draft ownership policy: PASS")
