"""Regression policy for MDPS-AUDIT2-202 application updater extraction isolation."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
network=(root/"src/networkAccess.cpp").read_text(encoding="utf-8")
header=(root/"src/networkAccess.h").read_text(encoding="utf-8")

assert "QString extractStagePath" in header
extract=network[network.index("void networkAccess::extractMediaDownloader"):network.index("QNetworkRequest networkAccess::networkRequest")]
assert '".mdps-app-update-extract-" + QUuid::createUuid().toString' in extract
assert "attemptInfo.exists() || attemptInfo.isSymLink()" in extract
assert "QDir().mkpath( m_md.extractStagePath )" in extract
assert '"-C",m_md.extractStagePath' in extract
assert '"-C",m_md.tmpPath' not in extract

finish=network[network.index("void networkAccess::emDownloader"):network.index("void networkAccess::failedToExtract")]
assert "QDir( md.extractStagePath ).filePath( baseName )" in finish
assert "utility::removeFolder( md.extractStagePath )" in finish
assert "QFileInfo( expectedExecutable ).isFile()" in finish
assert "utility::rename( extractedPath,md.finalPath )" in finish

# Every extraction failure/success path keeps cleanup rooted at the unique attempt.
assert finish.count("cleanupStage()") >= 4
print("Application updater unique extraction staging policy: PASS")
