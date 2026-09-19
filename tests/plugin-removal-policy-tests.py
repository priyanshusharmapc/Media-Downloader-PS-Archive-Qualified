"""Regression policy for MDPS-AUDIT2-150/151 plugin removal."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines.cpp").read_text(encoding="utf-8")
start=source.index("void engines::removeEngine")
end=source.index("QStringList engines::enginesList",start)
body=source[start:end]

assert "const auto name = engine->name()" in body
assert "const auto definitionError = utility::removeFile" in body
assert "if( !definitionError.isEmpty() )" in body
assert "return ;" in body[body.index("if( !definitionError.isEmpty() )"):body.index("this->removeEngineFromList")]
assert body.index("utility::removeFile") < body.index("this->removeEngineFromList")
assert "this->removeEngineFromList( name,id )" in body

# No dereference of result_ref is permitted after owner erasure.
after=body[body.index("this->removeEngineFromList( name,id )"):]
assert "engine->" not in after

assert "Plugin payload cleanup failed" in body
assert "entry.src()" in body and "entry.err()" in body

print("Plugin removal durability/lifetime policy: PASS")
