"""Regression policy for MDPS-AUDIT2-030/047 Recovery Package snapshot confinement."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
hdr=(root/"src/archive/archivecore.h").read_text(encoding="utf-8")
cpp=(root/"src/archive/archivecore.cpp").read_text(encoding="utf-8")
assert "validateSnapshot(const QString& packageDir,const QByteArray& manifestBytes) const" in hdr
snap=cpp[cpp.index("ValidationResult RecoveryImporter::validateSnapshot"):cpp.index("bool RecoveryImporter::normalizeVideo")]
assert "QDirIterator tree(packageDir" in snap
assert "QDir::AllEntries" in snap and "QDir::System" in snap
assert "detail::noLinks(path)" in snap
assert "info.isSymLink()" in snap
assert "!info.isFile()&&!info.isDir()" in snap
ingest=cpp[cpp.index("bool RecoveryImporter::ingest("):cpp.index("int RecoveryImporter::ingestPending")]
first_read=ingest.index('readBytes(QDir(absolute).filePath("manifest.json"),&manifestBytes')
validate=ingest.index("validateSnapshot(absolute,manifestBytes)")
parse=ingest.index("QJsonDocument::fromJson(manifestBytes)")
assert first_read >= 0 and first_read < validate < parse
assert ingest.count("validateSnapshot(absolute,manifestBytes)") >= 2
assert "Recovery package changed during normalization" in ingest
assert "QDir::AllEntries" in ingest and "QDir::System" in ingest
assert "evidence.fileInfo()" in ingest
assert "if(!info.isFile())" in ingest
print("Recovery Package immutable snapshot confinement policy: PASS")
