"""Regression policy for MDPS-AUDIT2-042 engine open-failure flow."""
from __future__ import annotations
import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
source = (p.parse_args().source_root / "src/networkAccess.cpp").read_text(encoding="utf-8")

start = source.index("void networkAccess::download( networkAccess::Opts opts ) const")
end = source.index("void networkAccess::downloadP", start)
body = source[start:end]
failure = body[body.index("}else{", body.index("opts.file.open")):]

assert "opts.reportDone()" not in failure, failure
assert "opts.reportFailed()" in failure, failure
assert "opts.networkError.add" in failure, failure
assert "this->finished( opts.move() )" in failure, failure
print("engine file-open failure continuation policy: PASS")
