from pathlib import Path
import argparse,json
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
r=p.parse_args().source_root
s=(r/"src/engines/deno.cpp").read_text(encoding="utf-8")
shipped=json.loads((r/"extensions/deno.json").read_text(encoding="utf-8"))
assert shipped["VersionArgument"]=="--version"
assert 'runContainedProcess( m,{ "--version" },QString(),10000 )' in s
assert 'mainObj.insert( "VersionArgument","--version" )' in s
assert '"-version"' not in s
print("Deno version-argument parity policy: PASS")
