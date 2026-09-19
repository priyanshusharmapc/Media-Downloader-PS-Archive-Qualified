"""Regression policy for MDPS-AUDIT2-193 Archive CSV formula neutralization."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")

start=source.index("QString csv(QString s)")
end=source.index("QString safeFilePart",start)
body=source[start:end]

assert 'QString("=+-@").contains(s.trimmed().front())' in body
assert 'QString("\\t\\r\\n").contains(s.front())' in body
assert "s.prepend('\'')" in body
assert 's.replace(\'"\',"\\"\\"")' in body or 's.replace(\'"\',"\\"\\"");' in body
assert "return '\"'+s+'\"'" in body

projection=source[source.index("bool Store::writeProjections"):source.index("bool Store::writeAllProjections")]
assert "quoted<<csv(x)" in projection
assert "mq<<csv(x)" in projection

print("Archive CSV formula neutralization policy: PASS")
