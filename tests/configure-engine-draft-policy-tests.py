"""Regression policy for MDPS-AUDIT2-149/153 Configure draft ownership."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/configure.h").read_text(encoding="utf-8")
cpp=(root/"src/configure.cpp").read_text(encoding="utf-8")

for member in ["m_textEncodingEngine","m_editOptionEngine","m_editOptionIdentity"]:
    assert member in hdr

pop=cpp[cpp.index("void configure::populateOptionsTable"):cpp.index("void configure::tabExited")]
assert "sameEncodingEngine" in pop
assert "m_settings.setTextEncoding( m_ui.lineEditConfigureTextEncoding->text(),m_textEncodingEngine )" in pop
assert "if( !sameEncodingEngine )" in pop
assert "m_textEncodingEngine = engineName" in pop

save=cpp[cpp.index("void configure::saveOptions"):cpp.index("void configure::setEngineOptions")]
assert "encodingEngine = m_textEncodingEngine.isEmpty() ? mm : m_textEncodingEngine" in save

assert "m_editOptionEngine = m_ui.cbConfigureEngines->currentText()" in cpp
assert "m_editOptionIdentity = m_tableDefaultDownloadOptions.stuffAt( row )" in cpp
assert "m_downloadEngineDefaultOptions.replace( m_editOptionIdentity,New )" in cpp
assert "current == oldObject" in cpp
assert "break ;" in cpp[cpp.index("void configure::downloadDefaultOptions::replace( const QJsonObject&"):cpp.index("QJsonObject configure::downloadDefaultOptions::addOpt")]
assert "m_ui.cbConfigureEngines->currentText() == m_editOptionEngine" in cpp

print("Configure per-engine draft ownership policy: PASS")

exit_body=cpp[cpp.index("void configure::tabExited"):cpp.index("void configure::updateEnginesList")]
assert "m_settings.setTextEncoding( m_ui.lineEditConfigureTextEncoding->text(),m_textEncodingEngine )" in exit_body
