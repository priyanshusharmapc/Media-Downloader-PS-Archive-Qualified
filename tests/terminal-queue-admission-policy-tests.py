"""Regression policy for MDPS-AUDIT2-133 terminal queue admission."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/tableWidget.cpp").read_text(encoding="utf-8")
start=source.index("int tableWidget::nextAvailableEntryToDownload")
end=source.index("void tableWidget::selectRow",start)
body=source[start:end]
assert "finishedStatus::notStarted" in body
assert "!this->runningOrFinishedWithSuccess" not in body
print("Terminal queue admission policy: PASS")
