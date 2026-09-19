"""Regression for MDPS-AUDIT2-104 proxy URL parsing."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines.cpp").read_text(encoding="utf-8")

start=source.index("QNetworkProxy engines::proxySettings::toQNetworkProxy")
end=source.index("void engines::proxySettings::setApplicationProxy",start)
body=source[start:end]
assert 'const QUrl url( input,QUrl::StrictMode )' in body
assert 'url.host().isEmpty()' in body
assert 'url.scheme().toLower()' in body
assert 'scheme == "socks5"' in body
assert 'scheme == "http" || scheme == "https"' in body
assert 'url.port( -1 )' in body
assert 'proxy.setHostName( url.host() )' in body
assert 'url.userName( QUrl::FullyDecoded )' in body
assert 'url.password( QUrl::FullyDecoded )' in body
assert "url.indexOf( ':' )" not in body

sstart=source.index("QString engines::proxySettings::toString")
send=source.index("QProcessEnvironment engines::engine::baseEngine::optionsEnvironment::update",sstart)
serializer=source[sstart:send]
assert "hostName.contains( ':' )" in serializer
assert 'hostName = "[" + hostName + "]"' in serializer
print("proxy URL semantic parser policy: PASS")
