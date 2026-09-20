"""Regression policy for MDPS-AUDIT2-156 shared folder opener."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/utility.cpp").read_text(encoding="utf-8")
start=s.index("void utility::openDownloadFolderPath"); end=s.index("QStringList utility::updateOptions",start)
body=s[start:end]
assert "url.trimmed().isEmpty()" in body
assert "QUrl::fromLocalFile( url )" in body
assert "QDesktopServices::openUrl( url )" not in body
print("Shared local-folder URL policy: PASS")
