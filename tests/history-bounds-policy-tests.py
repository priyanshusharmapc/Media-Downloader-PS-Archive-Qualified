from pathlib import Path
import argparse
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/settings.cpp").read_text(encoding="utf-8")
start=s.index("void settings::addToHistory")
end=s.index("void settings::addToplaylistRangeHistory",start)
body=s[start:end]
assert "if( max <= 0 )" in body
assert "history.clear()" in body
assert "while( history.size() > max )" in body
assert "input.isEmpty() || history.contains( input )" in body
assert "while( history.size() >= max )" in body
assert "history.size() == max" not in body
hstart=s.index("int settings::historySize()")
hend=s.index("QString settings::thumbnailTabName",hstart)
assert "size > 0 ? size : 0" in s[hstart:hend]
print("history bounds policy: PASS")
