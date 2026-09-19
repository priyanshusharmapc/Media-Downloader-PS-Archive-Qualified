"""Regression policy for Archive filter identity/state findings 181,185,186,189,190,191,192,199."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")

build=source[source.index("m_filter=new QComboBox"):source.index("rightLayout->addLayout(filterRow)")]
for stable in ["all","protected","needs_sync","missing","unavailable","removed","failed","interrupted"]:
    assert f'QStringLiteral("{stable}")' in build

start=source.index("void ArchiveTab::refreshTable()")
end=source.index("void ArchiveTab::refreshDetails()",start)
body=source[start:end]

# 181: display translation is not the filter identity.
assert "m_filter->currentData().toString()" in body
assert "m_filter->currentText()" not in body
assert 'knownFilters={"all","protected","needs_sync","missing","unavailable","removed","failed","interrupted"}' in body
assert 'if(!knownFilters.contains(filter))filter="all"' in body

# Item-level durable-state filters.
assert 'filter=="protected"' in body and 'status=="Protected"' in body
assert 'filter=="needs_sync"' in body and 'status=="Needs Sync"' in body
assert 'filter=="missing"' in body and 'c.video.state=="missing"' in body and 'c.audio.state=="missing"' in body and 'status=="Missing"' in body
assert 'filter=="unavailable"' in body and 'p.availability!="public"' in body
assert 'filter=="removed"' in body and 'p.membership=="removed"' in body
assert 'filter=="failed"' in body and 'status=="Failed"' in body and 'c.recoveryStatus=="failed"' in body
assert 'filter=="interrupted"' in body and 'status=="Interrupted"' in body and 'c.recoveryStatus=="interrupted"' in body

print("Archive stable filter identity/state policy: PASS")
