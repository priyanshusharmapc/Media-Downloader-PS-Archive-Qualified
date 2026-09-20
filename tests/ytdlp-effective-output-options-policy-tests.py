"""Regression policy for MDPS-AUDIT2-144 yt-dlp effective output-template metadata."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/yt-dlp.cpp").read_text(encoding="utf-8")
start=source.index("void yt_dlp::updateDownLoadCmdOptions")
end=source.index("void yt_dlp::updateGetPlaylistCmdOptions",start)
body=source[start:end]

base_call="engines::engine::baseEngine::updateDownLoadCmdOptions( s,e,extraOpts )"
assert body.count(base_call) == 1
scan='for( int m = s.ourOptions.size() - 1 ; m > -1 ; m-- )'
assert base_call in body and scan in body
assert body.index(base_call) < body.index(scan)
assert 'option == "-o" || option == "--output"' in body
assert 'option.startsWith( "--output=" )' in body
assert 'outputTemplate = option.mid( 9 )' in body
assert 'this->parseMetadata( mm,outputTemplate,"%(playlist_index)s",w )' in body
assert 'this->parseMetadata( mm,outputTemplate,"%(playlist_title)s",s.playlist_title )' in body
assert 's.ourOptions.append( mm )' in body
print("yt-dlp effective output-template metadata policy: PASS")
