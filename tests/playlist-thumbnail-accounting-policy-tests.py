"""Regression policy for MDPS-AUDIT2-056 thumbnail request accounting."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")

parse_start=s.index("bool playlistdownloader::parseJson")
result_start=s.index("void playlistdownloader::networkResult",parse_start)
parse=s[parse_start:result_start]
assert "m_networkRunning++" in parse
assert 'network.get( thumbnailUrl' in parse
assert 'networkDataSignal( { -1,media.move() } )' in parse

result_end=s.index("void playlistdownloader::networkData",result_start)
result=s[result_start:result_end]
assert "if( m_networkRunning > 0 )" in result
assert "m_networkRunning--" in result

data_end=s.index("void playlistdownloader::addTextToUi",result_end)
data=s[result_end:data_end]
assert "m_networkRunning--" not in data
assert "m_pendingRowMaterializations--" in data
assert "m_autoDownloadAfterMaterialization" in data

print("Playlist thumbnail request accounting policy: PASS")
