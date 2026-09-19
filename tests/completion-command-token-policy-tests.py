"""Regression policy for MDPS-AUDIT2-102 completion command token guards."""
from __future__ import annotations

import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
root = p.parse_args().source_root

settings = (root / "src/settings.cpp").read_text(encoding="utf-8")
utility = (root / "src/utility.h").read_text(encoding="utf-8")

s_start = settings.index("void settings::runCommandOnSuccessfulDownload")
s_end = settings.index("QString settings::commandWhenAllFinished", s_start)
s_body = settings[s_start:s_end]

u_start = utility.index("void updateFinishedState")
u_end = utility.index("Q_DECLARE_METATYPE", u_start)
u_body = utility[u_start:u_end]

assert "util::splitPreserveQuotes( m )" in s_body
assert "if( args.isEmpty() )" in s_body
assert s_body.index("if( args.isEmpty() )") < s_body.index("args.at( 0 )")

assert "util::splitPreserveQuotes( a )" in u_body
assert "if( args.isEmpty() )" in u_body
assert u_body.index("if( args.isEmpty() )") < u_body.index("args.takeAt( 0 )")

print("Completion command token guard policy: PASS")
