"""Regression policy for MDPS-AUDIT2-089 localization truthfulness."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
settings=(root/"src/settings.cpp").read_text(encoding="utf-8")
translator=(root/"src/translator.cpp").read_text(encoding="utf-8")
cmake=(root/"CMakeLists.txt").read_text(encoding="utf-8")

s0=settings.index("QString settings::localizationLanguage()")
s1=settings.index("bool settings::portableVersion()",s0)
body=settings[s0:s1]
assert 'language == "en_US" || QFile::exists' in body
assert 'available( persisted )' in body
assert 'm_settings.setValue( "Language",fallback )' in body

t0=translator.index("void translator::setLanguage")
t1=translator.index("void translator::setDefaultLanguage",t0)
tbody=translator[t0:t1]
assert 'if( e != "en_US" )' in tbody
assert 'candidate->load( e,m_pathLanguageFiles )' in tbody
assert 'm_settings.setLocalizationLanguage( "en_US" )' in tbody
assert "//???" not in tbody

print("Localization fallback truthfulness policy: PASS")

# macOS packaging must ship the complete compiled translation set rather than
# a manually maintained subset (canonical 089 packaging clarification).
assert 'file(GLOB MD_TRANSLATION_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.qm")' in cmake
assert 'file(COPY ${MD_TRANSLATION_FILES}' in cmake
for locale in ["bg_BG.qm","el_GR.qm","ko_KR.qm","uk_UA.qm"]:
    assert f"file( COPY translations/{locale}" not in cmake
