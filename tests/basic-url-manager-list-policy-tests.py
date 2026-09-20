"""Regression policy for MDPS-AUDIT2-109 URL-manager list routing."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
cpp=(root/"src/basicdownloader.cpp").read_text(encoding="utf-8")
hdr=(root/"src/basicdownloader.h").read_text(encoding="utf-8")
start=cpp.index("void basicdownloader::list()")
end=cpp.index("void basicdownloader::setContextMenuForDirectUrl",start)
body=cpp[start:end]
assert "this->defaultEngine( url )" in body
assert "this->defaultEngine()" not in body
rstart=cpp.index("void basicdownloader::listRequested")
rend=cpp.index("void basicdownloader::list()",rstart)
rbody=cpp[rstart:rend]
assert "const engines::engine& engine" in rbody
assert "cbEngineType->currentText()" not in rbody
assert "engine.mediaProperties" in rbody
assert "listRequested( m_listData,m_engine,m_id )" in cpp
assert "const engines::engine&,int" in hdr
print("Basic URL-manager list routing policy: PASS")
