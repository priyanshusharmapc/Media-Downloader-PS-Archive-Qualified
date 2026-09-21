"""Regression for MDPS-AUDIT2-096 rejected update payload cleanup."""
from pathlib import Path
import argparse
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/networkAccess.cpp").read_text(encoding="utf-8")

u=source[source.index("void networkAccess::uMediaDownloaderM"):source.index("void networkAccess::updateMediaDownloader( networkAccess::updateMDOptions",source.index("void networkAccess::uMediaDownloaderM"))]
assert u.count("utility::removeFile( md.tmpFile )") >= 2
assert "failedToRemove( m_appName,md.tmpFile" in u

f=source[source.index("void networkAccess::finished( networkAccess::Opts opts )"):source.index("void networkAccess::extractArchiveOuput",source.index("void networkAccess::finished( networkAccess::Opts opts )"))]
assert "QFileInfo rejectedPayload( opts.filePath )" in f
assert "rejectedPayload.exists() && rejectedPayload.isFile()" in f
assert "utility::removeFile( opts.filePath )" in f
assert "failedToRemove( engine.name(),opts.filePath" in f
assert f.index("rejectedPayload.exists() && rejectedPayload.isFile()") < f.index("utility::removeFile( opts.filePath )") < f.index("this->printVersion( opts.move(),opts.networkError.badDownload() )")
print("rejected temp cleanup policy: PASS")
