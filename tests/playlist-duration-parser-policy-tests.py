from pathlib import Path
import argparse
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/engines.cpp").read_text(encoding="utf-8")
start=s.index("int engines::engine::baseEngine::timer::toSeconds")
end=s.index("qint64 engines::engine::baseEngine::timer::elapsedTime()",start)
body=s[start:end]
assert "util::split( e,':',false )" in body
assert "minutes*60LL+seconds" in body
assert "hours * 3600LL + minutes * 60LL + seconds" in body
assert "std::numeric_limits< int >::max()" in body
assert "minutes>=60 || seconds>=60" in body
assert "seconds>=60" in body
assert "number < 0" in body
assert "toLongLong" in body
print("playlist duration parser policy: PASS")
