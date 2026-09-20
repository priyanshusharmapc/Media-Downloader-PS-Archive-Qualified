"""Regression policy for MDPS-AUDIT2-148 Basic completion notification routing."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/basicdownloader.cpp").read_text(encoding="utf-8")
i=s.index("desktopNotifyOnDownloadComplete()"); j=s.index("void basicdownloader::tabEntered",i)
body=s[i:j]
assert "desktopNotifyOnAllDownloadComplete()" in body
assert 'notifyOnAllDownloadComplete( "1 Download Complete" )' in body
assert 'else if( s.Settings().desktopNotifyOnAllDownloadComplete() )' not in body
print("Basic all-download notification routing policy: PASS")
