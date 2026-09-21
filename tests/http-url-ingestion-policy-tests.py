"""Regression policy for MDPS-AUDIT2-114 HTTP/HTTPS ingestion."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
u=(root/"src/utility.cpp").read_text(encoding="utf-8")
h=(root/"src/utility.h").read_text(encoding="utf-8")
t=(root/"src/tabmanager.cpp").read_text(encoding="utf-8")
b=(root/"src/batchdownloader.cpp").read_text(encoding="utf-8")
assert "bool utility::isHttpUrl" in u and "QUrl::StrictMode" in u
assert 'scheme.compare( "http",Qt::CaseInsensitive )' in u
assert 'scheme.compare( "https",Qt::CaseInsensitive )' in u
assert "bool isHttpUrl( const QString& )" in h
assert 'startsWith( "http" )' not in t
assert 'startsWith( "http" )' not in b
assert t.count("utility::isHttpUrl") >= 2
assert b.count("utility::isHttpUrl") >= 2
print("HTTP URL ingestion policy: PASS")

assert "utility::clipboardText().trimmed()" in b
assert "const auto candidate = it.trimmed()" in b
assert "items.add( candidate )" in b
assert "const auto candidate = text.trimmed()" in t
assert "clipboardData( candidate,true )" in t
