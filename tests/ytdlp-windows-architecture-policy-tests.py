"""Regression policy for MDPS-AUDIT2-152/172 Windows yt-dlp architecture selection."""
from __future__ import annotations
import argparse
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/engines/yt-dlp.cpp").read_text(encoding="utf-8")
assert 'return "yt-dlp_arm64.exe" ;' in source
start=source.index("static QString _WindowsBinaryName()")
end=source.index("static QString _Nicolaasjan",start)
selector=source[start:end]
assert "cpu.x86_32()" in selector
assert "cpu.aarch64()" in selector
assert "_Windows32BitBinaryName()" in selector
assert "_WindowsArm64BinaryName()" in selector
assert "_Windows64BitBinaryName()" in selector
assert "cpu.x86_64()" in selector
assert "return {} ;" in selector

start=source.index("void yt_dlp::checkIfBinaryExist")
end=source.index("static const char * _jsonFullArguments",start)
bootstrap=source[start:end]
assert "const auto binaryName = _WindowsBinaryName()" in bootstrap
assert "if( binaryName.isEmpty() )" in bootstrap
assert 'destPath += "/" + binaryName' in bootstrap
assert 'thirdPartyBinPath + "/ytdlp/" + binaryName' in bootstrap
assert 'thirdPartyBinPath + "/ytdlp/" + _Windows32BitBinaryName()' not in bootstrap

start=source.index("utility::addJsonCmd::entry::args yt_dlp::entryCmd")
end=source.index("utility::addJsonCmd::entry::args yt_dlp::entryCmdNightly",start)
mapping=source[start:end]
assert 'data.emplace_back( "aarch64",_WindowsArm64BinaryName() )' in mapping
print("Windows yt-dlp architecture selection policy: PASS")
