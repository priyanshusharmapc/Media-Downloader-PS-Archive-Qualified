"""Regression policy for MDPS-AUDIT2-069 per-engine cookie-source refresh."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/configure.cpp").read_text(encoding="utf-8")

label_start=source.index("void configure::setCookieSourceLabel( bool e )")
label_end=source.index("void configure::downloadExtension",label_start)
label=source[label_start:label_end]
assert "cookieBrowserName( name )" in label
assert "cookieBrowserTextFilePath( name )" in label
assert "getEngineByName( name )" in label
assert "setEnabled( enable && !e )" in label

opts_start=source.index("void configure::setEngineOptions")
opts_end=source.index("void configure::savePresetOptions",opts_start)
opts=source[opts_start:opts_end]
assert "setCookieSourceLabel( m_ui.cbCookieSource->isChecked() )" in opts
assert "cookieBrowserName( s->name() )" not in opts, (
    "engine switching must not force browser-cookie state regardless of active source")
print("per-engine cookie-source refresh policy: PASS")
