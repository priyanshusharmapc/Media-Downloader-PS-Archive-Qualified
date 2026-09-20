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
assert "data->append( chunk )" in body
assert "QLocalSocket::disconnected" in body
assert body.index("QLocalSocket::disconnected") < body.index("m_mainApp->hasEvent")
assert "m_mainApp->hasEvent( s->readAll() )" not in body
assert "&& m_mainApp" in body
assert "const qint64 maxEventBytes = 1024 * 1024" in body
assert "data->size() + chunk.size() > maxEventBytes" in body
assert "s->abort()" in body
assert "if( !*overflow && !data->isEmpty() && m_mainApp )" in body
print("Single-instance stream framing policy: PASS")
