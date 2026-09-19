"""Regression policy for MDPS-AUDIT2-178/179 Archive history and activity views."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
source=(p.parse_args().source_root/"src/archive/archivetab.cpp").read_text(encoding="utf-8")

details=source[source.index("void ArchiveTab::refreshDetails()"):source.index("void ArchiveTab::refreshActivity()")]
assert 'doc.object().value("item_key").toString()==key' in details
assert "line.contains(key)" not in details

activity=source[source.index("void ArchiveTab::refreshActivity()"):source.index("void ArchiveTab::updateActionState()",source.index("void ArchiveTab::refreshActivity()"))]
assert "for(int i=fileLines.size()-1;i>=0&&lines.size()<200;--i)" in activity
assert "if(lines.size()>=200)break;" in activity
assert "std::reverse(lines.begin(),lines.end())" in activity

print("Archive exact history/newest activity policy: PASS")
