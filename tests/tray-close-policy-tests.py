"""Regression for MDPS-AUDIT2-099 close-to-tray event handling."""
from pathlib import Path
import argparse
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/mainwindow.cpp").read_text(encoding="utf-8")
start=source.index("void MainWindow::closeEvent")
body=source[start:]
assert "QCloseEvent * event" in body
tray=body.index("if( m_showTrayIcon )")
ignore=body.index("event->ignore()",tray)
hide=body.index("this->hide()",ignore)
quit=body.index("this->quitApp()",hide)
assert tray < ignore < hide < quit
print("tray close policy: PASS")

# Tray residency must also be explicit at QApplication level and track runtime
# toggles, so implicit last-window shutdown cannot bypass quitApp().
assert "m_qApp.setQuitOnLastWindowClosed( !m_showTrayIcon )" in source
show=source[source.index("void MainWindow::showTrayIcon"):source.index("void MainWindow::keyPressEvent")]
assert "m_qApp.setQuitOnLastWindowClosed( !e )" in show
