"""Regression policy for MDPS-AUDIT2-058 pre-Qt-5.15 inactivity timeout."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
src=(p.parse_args().source_root/"src/utils/network_access_manager.hpp").read_text(encoding="utf-8")
handle=src[src.index("class handle"):src.index("template< typename Reply,typename Progress,typename Function >")]
assert "m_timeOut = timeOut" in handle
assert "m_timer.start( m_timeOut )" in handle
assert "void refreshTimer()" in handle
assert "QT_VERSION < QT_VERSION_CHECK( 5,15,0 )" in handle
assert "m_stopTimer" not in handle
setup=src[src.index("void setupReply"):src.index("template< typename Reply,typename Progress >",src.index("void setupReply")+10)]
assert "h.refreshTimer()" in setup
assert "h.stopTimer()" not in setup
print("Network inactivity timeout policy: PASS")
