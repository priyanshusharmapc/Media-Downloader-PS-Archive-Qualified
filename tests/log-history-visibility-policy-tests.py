"""Regression policy for MDPS-AUDIT2-080 log-window cross-mode visibility."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/logwindow.cpp").read_text(encoding="utf-8")

start=source.index("void logWindow::Show( bool s )")
end=source.index("void logWindow::clear()",start)
body=source[start:end]

visibility="m_ui->cbEnableDownloadHistory->setVisible( m_showDownloadHistory )"
assert visibility in body, (
    "history checkbox visibility must be recomputed directly from the active mode")
assert body.index(visibility) < body.index("if( m_showDownloadHistory )"), (
    "visibility must be restored before history/ordinary branch-specific state")
assert "setVisible( false )" not in body, (
    "one-way hiding must not leak across reused log-window modes")
print("log-window cross-mode history visibility: PASS")
