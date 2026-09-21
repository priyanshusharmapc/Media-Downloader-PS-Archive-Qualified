"""Regression policy for MDPS-AUDIT2-071 download-folder filesystem type."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/settings.cpp").read_text(encoding="utf-8")

start=source.index("QString settings::downloadFolder( const QString& defaultPath")
end=source.index("QString settings::downloadFolderImp",start)
body=source[start:end]

assert "QFileInfo( resolved ).isDir()" in body, (
    "download-folder validation must require a directory")
assert "if( QFile::exists( m ) )" not in body, (
    "generic existence must not admit a regular file as a directory root")
assert 'const auto configured = m_settings.value( "DownloadFolder" ).toString()' in body
assert 'if( configured.startsWith( defaultMarker ) )' in body
assert 'return defaultPath' in body
assert 'm_settings.setValue( "DownloadFolder"' not in body.split("const auto configured",1)[1], (
    "offline configured destinations must not be overwritten by fallback")
print("download-folder filesystem type policy: PASS")
