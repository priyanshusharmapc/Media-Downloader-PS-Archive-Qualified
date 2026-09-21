"""Regression policy for AUDIT2-184/194/195 source admission."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
tab=(root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")
core=(root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")
add=tab[tab.index("void ArchiveTab::addPlaylist()"):tab.index("void ArchiveTab::removePlaylist()")]
key=core[core.index("QString sourceKeyFromUrl"):core.index("QString videoIdFromUrl",core.index("QString sourceKeyFromUrl"))]
assert ".trimmed()" in add
assert 'const auto canonicalUrl=QStringLiteral("https://www.youtube.com/playlist?list=")+key' in add
assert "source.url=canonicalUrl" in add
assert "existing.key==key" in add
assert "m_sources->setCurrentRow(i)" in add
assert "sources.append(source)" not in add
assert "operationScanOrSync({source},true,true)" in add
assert 'url.scheme()!="https"&&url.scheme()!="http"' in key
assert "!url.userInfo().isEmpty()" in key
assert 'host=="youtube.com"||host.endsWith(".youtube.com")' in key
assert 'queryItemValue("list")' in key
assert "sourceKeySafe" in key
print("Archive source admission policy: PASS")
