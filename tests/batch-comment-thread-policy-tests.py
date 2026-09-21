"""Regression policy for MDPS-AUDIT2-078 complete comment-thread export."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/batchdownloader.cpp").read_text(encoding="utf-8")

start=source.index("static QJsonArray _saveComments")
end=source.index("template< typename Array,typename Table >",start)
body=source[start:end]

assert "QHash< QString,int > byId" in body
assert body.index("byId.insert") < body.index("parentIndex = byId.value"), (
    "all IDs must be indexed before parent attachment")
assert "nodes[ parentIndex ].children.append( i )" in body
assert "std::function< QJsonObject( int ) > build" in body
assert 'out.insert( "text replies",replies )' in body
assert "parentIndex < 0" in body, "missing parents must be retained rather than dropped"
assert "for( int i = 0 ; i < nodes.size() ; ++i )" in body.split(
    "// A malformed parent cycle has no natural root.",1)[1]
assert "if( !emitted[ i ] )" in body
print("complete nested comment-thread export policy: PASS")
