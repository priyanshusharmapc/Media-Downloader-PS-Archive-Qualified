"""Regression for MDPS-AUDIT2-045 download-folder identity preservation."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/settings.cpp").read_text(encoding="utf-8")
start=source.index("QString settings::downloadFolder( const QString& defaultPath")
end=source.index("QString settings::downloadFolderImp",start)
body=source[start:end]

assert 'if( !m_settings.contains( "DownloadFolder" ) )' in body
assert 'm_settings.setValue( "DownloadFolder",defaultMarker )' in body
assert 'const auto configured = m_settings.value( "DownloadFolder" ).toString()' in body
assert 'if( QFileInfo( resolved ).isDir() )' in body
assert 'configured.startsWith( defaultMarker )' in body
assert 'Configured download folder is unavailable; using the default folder for this operation' in body

# No unavailable-path branch may persistently reset the user's configured root.
writes=[line.strip() for line in body.splitlines() if 'm_settings.setValue( "DownloadFolder"' in line]
assert writes == ['m_settings.setValue( "DownloadFolder",defaultMarker ) ;']
print("download-folder preservation policy: PASS")
