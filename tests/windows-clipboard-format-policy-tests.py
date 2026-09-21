"""Regression policy for MDPS-AUDIT2-062 Win32 clipboard decoding."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/utility.cpp").read_text(encoding="utf-8")
start=source.index("QString utility::windowsGetClipBoardText")
end=source.index("void utility::windowsSetDarkModeTitleBar",start)
body=source[start:end]

assert "utility::Qt6Version()" not in body
assert "LPTSTR" not in body
assert "CF_UNICODETEXT" in body and "CF_TEXT" in body
assert "QString::fromWCharArray" in body
assert "QString::fromLocal8Bit" in body
assert body.index("CF_UNICODETEXT") < body.index("CF_TEXT")
print("Win32 clipboard format decoding policy: PASS")
