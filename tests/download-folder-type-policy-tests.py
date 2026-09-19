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

assert "QFileInfo( m ).isDir()" in body, (
    "download-folder validation must require a directory")
assert "if( QFile::exists( m ) )" not in body, (
    "generic existence must not admit a regular file as a directory root")
assert 'm_settings.setValue( "DownloadFolder",mm )' in body, (
    "existing invalid-path fallback behavior must remain explicit")
print("download-folder filesystem type policy: PASS")
