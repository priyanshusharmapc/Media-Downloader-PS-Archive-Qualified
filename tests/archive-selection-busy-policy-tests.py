"""Regression policy for MDPS-AUDIT2-187/188 Archive refresh selection and busy gating."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")

refresh=source[source.index("void ArchiveTab::refreshTable()"):source.index("void ArchiveTab::refreshDetails()")]
assert "const auto selectedKey=selectedItemKey()" in refresh
assert "p.itemKey==selectedKey" in refresh
assert "m_table->selectRow(selectedRow)" in refresh

state=source[source.index("void ArchiveTab::updateActionState()"):source.index("void ArchiveTab::setBusy",source.index("void ArchiveTab::updateActionState()"))]
assert "const bool ready=m_controlsEnabled&&!m_busy" in state
assert "m_more" in state
assert "widget->setEnabled(ready)" in state

print("Archive selection preservation/busy More gating policy: PASS")
