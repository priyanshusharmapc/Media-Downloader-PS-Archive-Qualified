"""Regression policy for MDPS-AUDIT2-147 Configure engine visibility reset."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/configure.cpp").read_text(encoding="utf-8")
start=s.index("void configure::populateOptionsTable"); end=s.index("void configure::tabExited",start)
body=s[start:end]
reset=body.index("lineEditConfigureTextEncoding->setVisible( true )")
deno=body.index('if( s.name() == "deno" )')
assert reset < deno
assert "cbDenoEnableAutoDownload->setVisible( false )" in body[:deno]
assert 'labelConfigureTextEncoding->setText( tr( "Text Encoding" ) )' in body[:deno]
print("Configure engine visibility reset policy: PASS")
