"""Regression policy for MDPS-AUDIT2-132 end-of-queue command dispatch."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
header=(root/"src/tableWidget.h").read_text(encoding="utf-8")
table=(root/"src/tableWidget.cpp").read_text(encoding="utf-8")
utility=(root/"src/utility.h").read_text(encoding="utf-8")

assert "bool allEntriesTerminal( int firstRow = 0 ) const" in header
start=table.index("bool tableWidget::allEntriesTerminal")
end=table.index("bool tableWidget::finishedWithSuccess",start)
body=table[start:end]
assert "finishedWithSuccess" in body
assert "finishedWithError" in body
assert "finishedCancelled" in body
assert "return m_table.rowCount() > firstRow" in body

start=utility.index("void updateFinishedState")
end=utility.index("Q_DECLARE_METATYPE",start)
body=utility[start:end]
assert 'const auto firstJobRow = tabName == "playlist" ? 1 : 0' in body
assert "f.done() && table.allEntriesTerminal( firstJobRow )" in body
assert body.index("table.setRunningState") < body.index("allEntriesTerminal")
assert "if( !args.isEmpty() )" in body
print("All-finished command terminal transition policy: PASS")
