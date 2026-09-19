"""Regression policy for MDPS-AUDIT2-127 downloader option drafts."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
cases=[
 ("src/basicdownloader.cpp","void basicdownloader::tabEntered","lineEditOptions"),
 ("src/batchdownloader.cpp","void batchdownloader::tabEntered","lineEditBDUrlOptions"),
 ("src/playlistdownloader.cpp","void playlistdownloader::tabEntered","lineEditPLUrlOptions"),
]
for path,marker,editor in cases:
 s=(root/path).read_text(encoding="utf-8")
 start=s.index(marker); end=s.index("tabExited",start)
 body=s[start:end]
 assert f"!m_ui.{editor}->isModified()" in body, (path,body)
 assert "lastUsedOption" in body
print("Downloader option draft preservation policy: PASS")
