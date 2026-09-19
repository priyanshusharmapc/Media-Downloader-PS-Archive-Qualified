"""Regression for MDPS-AUDIT2-097 RegGetValueW byte-count decoding."""
from pathlib import Path
import argparse

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/utility.cpp").read_text(encoding="utf-8")
start=source.index("QString getExePath()")
end=source.index("buffer getSubKey",start)
body=source[start:end]

assert "std::array< wchar_t,4096 > value{}" in body
assert "DWORD bytes = static_cast< DWORD >( sizeof( value ) )" in body
assert "RegGetValueW" in body and "value.data(),&bytes" in body
assert "bytes / sizeof( wchar_t )" in body
assert "QString::fromWCharArray( value.data(),chars )" in body
assert "subKey.qdata()" not in body
print("registry byte-count decoding policy: PASS")
