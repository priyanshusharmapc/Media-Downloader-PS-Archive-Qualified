"""Regression policy for MDPS-AUDIT2-068 URL-manager engine identity."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/configure.cpp").read_text(encoding="utf-8")

start=source.index("connect( m_ui.cbConfigureEnginesUrlManager,cc")
end=source.index("connect( m_ui.pbConfigureQuit",start)
handler=source[start:end]

assert "cbConfigureEnginesUrlManager->itemText( index )" in handler, (
    "URL-manager handler must read the engine from its own combo box")
assert "cbConfigureEngines->itemText( index )" not in handler, (
    "URL-manager selection must not depend on the general engine combo order")
assert "setEngineOptions( m,engineOptions::url )" in handler
print("URL-manager engine selection identity: PASS")
