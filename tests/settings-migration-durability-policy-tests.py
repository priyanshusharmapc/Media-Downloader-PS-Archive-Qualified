"""Regression for MDPS-AUDIT2-046 settings migration durability."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/settings.cpp").read_text(encoding="utf-8")
start=source.index("std::unique_ptr< QSettings > settings::init()")
end=source.index("#if QT_VERSION >=",start)
body=source[start:end]

copy=body.index("newSettings->setValue")
sync=body.index("newSettings->sync()",copy)
status=body.index("newSettings->status() == QSettings::NoError",sync)
verify=body.index("QSettings verify",status)
compare=body.index("verify.value( it ) != oldSettings.value( it )",verify)
clear=body.index("oldSettings.clear()",compare)
assert copy < sync < status < verify < compare < clear
assert "return std::make_unique< QSettings >( "media-downloader","media-downloader" )" in body
assert "QFile::remove( failedPath )" in body
print("settings migration durability policy: PASS")
