"""Regression policy for MDPS-AUDIT2-116 Windows update promotion transaction."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/utility.cpp").read_text(encoding="utf-8")
start=source.index("bool utility::startedUpdatedVersion")
end=source.index("bool utility::platformIsLikeWindows",start)
body=source[start:end]

assert "const auto hadCurrentUpdate" in body
assert "if( !dir.rename( update,updated_old ) )" in body
assert "if( !dir.rename( update_new,update ) )" in body
assert "hadCurrentUpdate && !dir.rename( updated_old,update )" in body
first=body.index("if( !dir.rename( update,updated_old ) )")
second=body.index("if( !dir.rename( update_new,update ) )")
exe=body.index('QString exePath = update + "/media-downloader.exe"')
assert first < second < exe
print("Windows update promotion transaction policy: PASS")
