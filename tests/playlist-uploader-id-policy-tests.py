"""Regression policy for MDPS-AUDIT2-105 playlist uploader identity propagation."""
from __future__ import annotations

import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
root=p.parse_args().source_root
source=(root/"src/tableWidget.h").read_text(encoding="utf-8")
engines=(root/"src/engines.h").read_text(encoding="utf-8")
ytdlp=(root/"src/engines/yt-dlp.cpp").read_text(encoding="utf-8")

start = source.index("template< typename MediaProperties >")
end = source.index("entry move()", start)
body = source[start:end]

assert "playlist_id( media.playlist_id() )" in body
assert "playlist_uploader_id( media.playlist_uploader_id() )" in body
assert "playlist_uploader_id( media.playlist_id() )" not in body

print("Playlist uploader identity propagation policy: PASS")

# Lock the corrected identity through the command-generation consumer too.
assert "playlist_uploader_id( ent.playlist_uploader_id )" in engines
assert '"%(playlist_uploader_id)s",s.playlist_uploader_id' in ytdlp.replace(" ","")
assert '"%(playlist_uploader_id)s",s.playlist_id' not in ytdlp.replace(" ","")
