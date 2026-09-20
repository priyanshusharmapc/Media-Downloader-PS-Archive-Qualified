"""Regression policy for MDPS-AUDIT2-041 promoted engine permissions."""
from __future__ import annotations
import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
root = p.parse_args().source_root
source = (root / "src/networkAccess.cpp").read_text(encoding="utf-8")

needle = "const auto m = promoteUpdatePath( opts.file.src(),opts.exeBinPath,&cleanupWarning )"
start = source.index(needle)
end = source.index("}else{", start)
success = source[start:end]

assert "utility::setPermissions( opts.exeBinPath )" in success, success
assert "utility::setPermissions( opts.file.src() )" not in success, success
print("promoted engine permission target policy: PASS")


# MDPS-AUDIT2-040: archives are extracted away from the live engine tree and
# both archive and standalone promotion use rollback-capable commit helpers.
assert '.mdps-update-stage-' in source, 'archive extraction is not staged'
assert 'promoteUpdateDirectoryContents( opts.updateStagePath,opts.tempPath' in source
assert 'promoteUpdatePath( opts.file.src(),opts.exeBinPath' in source
archive_start = source.index('void networkAccess::extractArchive( networkAccess::Opts opts ) const')
archive_end = source.index('void networkAccess::post( const QString& engineName', archive_start)
archive = source[archive_start:archive_end]
assert 'deleteEngineBinFolder( opts.tempPath )' not in archive, archive
assert 'engine.removeFiles( { opts.exeBinPath }' not in archive, archive
assert '"-C",opts.updateStagePath' in archive or 'opts.filePath,"-d",opts.updateStagePath' in archive
finished_start = source.index('void networkAccess::finished( networkAccess::Opts opts ) const')
finished_end = source.index('void networkAccess::extractArchiveOuput', finished_start)
finished = source[finished_start:finished_end]
assert 'utility::removeFile( opts.exeBinPath )' not in finished, finished
assert 'utility::removeFolder( opts.exeBinPath )' not in finished, finished
print('engine update rollback/staging policy: PASS')

# Staged archive layout is validated before rollback-capable promotion starts.
output_start = source.index("void networkAccess::extractArchiveOuput")
output_end = source.index("void networkAccess::postStartDownloading", output_start)
output = source[output_start:output_end]
assert "expectedRelative" in output and "stagedExecutable" in output
assert "QFileInfo( stagedExecutable ).isFile()" in output
assert "Extracted update is missing the expected executable" in output
assert output.index("QFileInfo( stagedExecutable ).isFile()") < output.index("promoteUpdateDirectoryContents")
