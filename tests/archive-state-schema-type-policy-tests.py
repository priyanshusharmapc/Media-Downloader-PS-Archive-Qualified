"""Regression policy for MDPS-AUDIT2-021 persisted-state type validation."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivesafety.h").read_text(encoding="utf-8")

for helper in ["stringField","integerField","stringArrayField","representationShape"]:
    assert f"bool {helper}" in source

start=source.index("inline bool arrayShape")
end=source.index("inline bool readArray",start)
body=source[start:end]

for field in ["title","uploader","original_url","availability","first_seen","last_seen","recovery_status","metadata_path"]:
    assert f'"{field}"' in body
assert '"user_tags"' in body and "stringArrayField" in body
for field in ["origin","verified_at","error"]:
    assert f'"{field}"' in source[source.index("inline bool representationShape"):start]
for field in ["entry_key","url","position","last_position"]:
    assert f'"{field}"' in body
assert "integerField(o,"position"" in body
assert "integerField(o,"last_position"" in body

print("Persisted Archive state type validation policy: PASS")

# Integer type validation also enforces the model's only allowed negative
# sentinel; arbitrary negative positions must fail closed.
assert 'o.value("position").toInt() < -1' in body
assert 'o.value("last_position").toInt() < -1' in body
assert '"Invalid playlist position range"' in body
