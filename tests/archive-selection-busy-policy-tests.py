"""Regression policy for AUDIT2-187/188 selection preservation and busy gating."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")
refresh=source[source.index("void ArchiveTab::refreshTable()"):source.index("void ArchiveTab::refreshDetails()")]
assert "const auto selectedEntry=selectedEntryKey()" in refresh
assert "p.entryKey==selectedEntry" in refresh
assert "m_table->selectRow(selectedRow)" in refresh
state=source[source.index("void ArchiveTab::updateActionState()"):source.index("void ArchiveTab::setBusy")]
assert "m_controlsEnabled&&!m_busy&&m_ready&&m_stateReadable" in state
assert "m_more" in state and "globalControls" in state
assert "sourceSelected" in state
print("Archive selection preservation/busy gating policy: PASS")
