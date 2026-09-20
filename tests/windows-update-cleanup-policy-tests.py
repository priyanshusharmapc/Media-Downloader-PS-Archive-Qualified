"""Regression policy for MDPS-AUDIT2-201 updater cleanup ownership."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
utility=(root/"src/utility.cpp").read_text(encoding="utf-8")
settings=(root/"src/settings.cpp").read_text(encoding="utf-8")

start=utility.index("bool utility::isOwnedUpdateCleanupPath")
end=utility.index("bool utility::startedUpdatedVersion",start)
helper=utility[start:end]

assert "!runningUpdated" in helper
assert "currentExecutable.trimmed().isEmpty()" in helper
assert 'name.startsWith( "update-" )' in helper
assert "candidateInfo.isSymLink()" in helper
assert "rootInfo.canonicalFilePath()" in helper
assert "candidateInfo.canonicalFilePath()" in helper
assert "QDir( rootCanonical ).absoluteFilePath( name )" in helper
assert 'QDir( updateDirCanonical ).absoluteFilePath( "media-downloader.exe" )' in helper
assert "executableInfo.canonicalFilePath()" in helper
assert "candidateCanonical.compare( expected,Qt::CaseInsensitive ) == 0" in helper

init=settings[settings.index("void settings::init_done"):settings.index("void settings::setTabNumber")]
assert "pathToOldUpdatedVersion()" in init
assert "m_options.runningUpdated()" in init
assert "QCoreApplication::applicationFilePath()" in init
assert "utility::isOwnedUpdateCleanupPath( configPath,candidate,runningUpdated,currentExecutable )" in init
assert init.index("isOwnedUpdateCleanupPath") < init.index("QDir( candidate ).removeRecursively()")
assert "QDir( m ).removeRecursively()" not in init

print("Windows updater cleanup ownership policy: PASS")
