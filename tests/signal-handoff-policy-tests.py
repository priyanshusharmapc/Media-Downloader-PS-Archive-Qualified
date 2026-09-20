"""Regression policy for MDPS-AUDIT2-053 signal-safe shutdown handoff."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
cpp=(root/"src/mainwindow.cpp").read_text(encoding="utf-8")
hdr=(root/"src/mainwindow.h").read_text(encoding="utf-8")

assert "volatile std::sig_atomic_t MainWindow::m_signalPending" in cpp
assert "m_signalPending = sig" in cpp
handler=cpp[cpp.index("void MainWindow::signalHandler"):cpp.index("void MainWindow::setUpSignal")]
assert "quitApp()" not in handler
assert "QCoreApplication::quit" not in handler
setup=cpp[cpp.index("void MainWindow::setUpSignals"):cpp.index("void MainWindow::signalHandler")]
assert "SIGTERM,SIGINT" in setup
assert "SIGSEGV" not in setup and "SIGABRT" not in setup
assert "QTimer" in cpp
assert "this->quitApp()" in cpp[cpp.index("signalTimer"):cpp.index("signalTimer->start()")+40]
assert "static volatile std::sig_atomic_t m_signalPending" in hdr
print("Signal-safe shutdown handoff policy: PASS")

# Handler installation itself is checked, so graceful-shutdown support cannot
# silently disappear when std::signal rejects a registration.
assert "std::signal( sig,MainWindow::signalHandler ) != SIG_ERR" in cpp
assert "if( !MainWindow::setUpSignal( SIGTERM,SIGINT ) )" in cpp
assert "static bool setUpSignal( int )" in hdr

assert "#include <iostream>" in cpp
assert "static bool setUpSignal( Int sig,INTS ... sigs )" in hdr
assert "return MainWindow::setUpSignal( sigs ... ) && current" in hdr
