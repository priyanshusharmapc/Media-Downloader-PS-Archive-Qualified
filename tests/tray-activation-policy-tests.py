"""Regression policy for MDPS-AUDIT2-086 tray activation semantics."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/mainwindow.cpp").read_text(encoding="utf-8")
start=s.index("QSystemTrayIcon::activated")
end=s.index("if( m_showTrayIcon )",start)
body=s[start:end]
assert "ActivationReason reason" in body
assert "reason != QSystemTrayIcon::Trigger" in body
assert body.index("reason != QSystemTrayIcon::Trigger") < body.index("this->isVisible()")
print("Tray activation reason policy: PASS")
