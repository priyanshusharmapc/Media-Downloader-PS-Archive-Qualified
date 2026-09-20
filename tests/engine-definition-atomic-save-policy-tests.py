"""Regression for MDPS-AUDIT2-043 / 175 engine-definition admission ordering."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines.cpp").read_text(encoding="utf-8")
start=source.index("QString engines::addEngine( const QByteArray&")
end=source.index("void engines::removeEngine",start)
body=source[start:end]

assert "QSaveFile file( path )" in body
assert "file.open( QIODevice::WriteOnly )" in body
assert "file.write( data ) != data.size()" in body
assert "!file.commit()" in body
assert "QIODevice::Truncate" not in body
assert "utility::waitForOneSecond()" not in body

# Semantic admission must happen before the QSaveFile commit. This preserves
# an existing working definition when the replacement is syntactically valid
# JSON but not a usable engine.
assert "auto candidate =" in body
assert "candidate->exePath().isEmpty()" in body
assert "Rejected engine definition before persistence" in body
assert body.index("auto candidate =") < body.index("QSaveFile file( path )")
assert body.index("candidate->exePath().isEmpty()") < body.index("file.commit()")

# Derived definitions must be validated with the same composition rules used
# by getEngineByPath(), not as raw standalone JSON.
assert "yt_dlp::cmdNightly" in body
assert "yt_dlp::cmdFfmpeg" in body
assert "yt_dlp::cmdAria2C" in body

# Do not solve failed custom installs by forbidding same-name replacement,
# because this shared function is also the automatic definition-refresh path.
assert "Plugin definition already exists" not in body
assert "QFileInfo::exists( path )" not in body

# The exact prevalidated object is admitted after durable commit.
assert "engineAdd( extensionFileName,candidate.move(),id )" in body
print("engine definition pre-admission + atomic replacement policy: PASS")
