from pathlib import Path
import argparse
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/engines/bun.cpp").read_text(encoding="utf-8")
r=s[s.index("void bun::remove"):]
assert 'utility::platformIsWindows() ? "bun.exe" : "bun"' in r
assert 'enginePath.binPath( "bun" )' not in r
print("Bun removal filename parity policy: PASS")
