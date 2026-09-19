"""Regression policy for MDPS-AUDIT2-184/194/195 Archive source admission."""
from __future__ import annotations

import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
root = p.parse_args().source_root

tab = (root / "src/archive/archivetab.cpp").read_text(encoding="utf-8")
core = (root / "src/archive/archivecore.cpp").read_text(encoding="utf-8")

add_start = tab.index("void ArchiveTab::addPlaylist()")
add_end = tab.index("void ArchiveTab::removePlaylist()", add_start)
add = tab[add_start:add_end]

key_start = core.index("QString sourceKeyFromUrl")
key_end = core.index("QString videoIdFromUrl", key_start)
key = core[key_start:key_end]

# 194: pasted whitespace and equivalent URL forms converge to one durable URL.
assert "getText(" in add and ".trimmed()" in add
assert 'const auto canonicalUrl=QStringLiteral("https://www.youtube.com/playlist?list=")+key' in add
assert "source.url=canonicalUrl" in add
assert '{"url",canonicalUrl}' in add

# 195: arbitrary text/paths/unsupported schemes cannot become durable sources.
assert 'url.scheme()!="https"&&url.scheme()!="http"' in key
assert '!url.userInfo().isEmpty()' in key
assert 'host=="youtube.com"||host.endsWith(".youtube.com")' in key
assert 'queryItemValue("list")' in key
assert "sourceKeySafe" in key
assert "if(key.isEmpty())" in add

# 184: equivalent admitted sources are de-duplicated by canonical playlist key.
assert "store.loadSources" in add
assert "source.key==key" in add
assert "already registered" in add
assert "m_sources->setCurrentRow(i)" in add
assert add.index("source.key==key") < add.index("sources.append(source)")

print("Archive source admission policy: PASS")
