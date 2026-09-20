"""Regression policy for MDPS-AUDIT2-118 subscription action identity."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
h=(root/"src/playlistdownloader.h").read_text(encoding="utf-8")
c=(root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")
start=c.index("void playlistdownloader::plSubscription"); end=c.index("void playlistdownloader::download()",start)
body=c[start:end]
assert "entry( const QJsonObject& obj )" in h
assert "ac->setData" in body
assert "subscription::entry::toObject( s.UiName(),s.url(),s.options() )" in body
assert "QJsonDocument::fromJson( ac->data().toByteArray() )" in body
assert "if( e.url() == s )" not in body
print("Subscription action identity policy: PASS")
