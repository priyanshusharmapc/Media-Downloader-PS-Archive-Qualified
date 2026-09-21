"""Regression policy for MDPS-AUDIT2-066 Linux gateway-proxy fallback."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/proxy.cpp").read_text(encoding="utf-8")

start=source.index("static void _get_proxy_from_gateway_linux")
end=source.index("static void _get_proxy_from_gateway_win",start)
body=source[start:end]

open_guard=body.index("if( !file.open( QIODevice::ReadOnly ) )")
fallback=body.index("ctx.setNetworkProxy( firstTime )",open_guard)
early_return=body.index("return ;",open_guard)
assert open_guard < fallback < early_return, (
    "route-table open failure must explicitly apply the no-gateway proxy fallback before returning")
print("Linux gateway-proxy open-failure fallback: PASS")
