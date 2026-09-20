"""Regression policy for stable Archive filter identity and state predicates."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")
build=source[source.index("m_filter=new QComboBox"):source.index("rightLayout->addLayout(filterRow)")]
for stable in ["all","protected","needs_sync","missing","unavailable","removed","failed","interrupted"]:
    assert f'QStringLiteral("{stable}")' in build
body=source[source.index("void ArchiveTab::refreshTable()"):source.index("void ArchiveTab::refreshDetails()")]
assert "m_filter->currentData().toString()" in body
assert "m_filter->currentText()" not in body
assert 'if(!knownFilters.contains(filter))filter="all"' in body
assert 'filter=="protected"' in body and 'status=="Protected"' in body
assert 'filter=="needs_sync"' in body and 'status=="Verification Stale"' in body
assert 'filter=="missing"' in body and 'video.label=="Missing"' in body and 'audio.label=="Missing"' in body
assert 'filter=="unavailable"' in body and 'p.availability!="public"' in body
assert 'filter=="removed"' in body and 'p.membership=="removed"' in body
assert 'filter=="failed"' in body and 'item.recoveryStatus=="failed"' in body
assert 'filter=="interrupted"' in body and 'item.recoveryStatus=="interrupted"' in body
print("Archive stable filter identity/state policy: PASS")
