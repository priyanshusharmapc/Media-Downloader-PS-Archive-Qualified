"""Regression policy for MDPS-AUDIT2-061 single-instance stream framing."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
s=(p.parse_args().source_root/"src/utils/single_instance.hpp").read_text(encoding="utf-8")
start=s.index("QLocalServer::newConnection")
end=s.index("m_localServer.listen",start)
body=s[start:end]
assert "std::make_shared< QByteArray >" in body
assert "QLocalSocket::readyRead" in body
assert "data->append( s->readAll() )" in body
assert "QLocalSocket::disconnected" in body
assert body.index("QLocalSocket::disconnected") < body.index("m_mainApp->hasEvent")
assert "m_mainApp->hasEvent( s->readAll() )" not in body
print("Single-instance stream framing policy: PASS")

# Stream framing is bounded as well as complete.
assert "const qsizetype maxEventBytes = 1024 * 1024" in body
assert "data->size() + chunk.size() > maxEventBytes" in body
assert "s->abort()" in body
assert "if( !*overflow && !data->isEmpty() )" in body
