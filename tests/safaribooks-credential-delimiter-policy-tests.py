"""Regression for MDPS-AUDIT2-092 SafariBooks credential delimiter handling."""
from pathlib import Path
import argparse

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/safaribooks.cpp").read_text(encoding="utf-8")
start=source.index("void safaribooks::sendCredentials")
end=source.index("static bool _credential_line",start)
body=source[start:end]

assert "credentials.indexOf( ':' )" in body
assert "credentials.left( separator )" in body
assert "credentials.mid( separator + 1 )" in body
assert "util::split( credentials,':',true )" not in body
print("SafariBooks credential delimiter policy: PASS")
