"""Regression policy for MDPS-AUDIT2-070 maximum concurrent-download validation."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
configure=(root/"src/configure.cpp").read_text(encoding="utf-8")
settings=(root/"src/settings.cpp").read_text(encoding="utf-8")

save_start=configure.index("void configure::saveOptions()")
save_end=configure.index("void configure::setEngineOptions",save_start)
save=configure[save_start:save_end]
assert "toInt( &maxDownloadsOk )" in save
assert "maxDownloadsOk && maxDownloads > 0 ? maxDownloads : 4" in save

read_start=settings.index("size_t settings::maxConcurrentDownloads()")
read_end=settings.index("bool settings::darkTheme()",read_start)
read=settings[read_start:read_end]
assert "m > 0 ? m : 4" in read
assert "static_cast< size_t >( m )" not in read

set_start=settings.index("void settings::setMaxConcurrentDownloads( int s )")
set_end=settings.index("void settings::setDownloadFolder",set_start)
setter=settings[set_start:set_end]
assert "s > 0 ? s : 4" in setter
print("maximum concurrent-download validation policy: PASS")
