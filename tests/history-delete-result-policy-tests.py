"""Regression for MDPS-AUDIT2-098 truthful serialized history clearing."""
from pathlib import Path
import argparse
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
logger=(root/"src/logger.cpp").read_text(encoding="utf-8")
window=(root/"src/logwindow.cpp").read_text(encoding="utf-8")
utility=(root/"src/utility.cpp").read_text(encoding="utf-8")
start=logger.index("bool Logger::clearDownloadHistory()")
end=logger.index("void Logger::reTranslateLogWindow",start)
body=logger[start:end]
assert "return utility::archiveData::clearHistory( e )" in body
ustart=utility.index("bool utility::archiveData::clearHistory")
uend=utility.index("utility::archiveData::archiveData(",ustart)
ubody=utility[ustart:uend]
assert "guardHistoryFile()" in ubody and "unGuardHistoryFile()" in ubody
assert 'QLockFile lock( filePath + ".lock" )' in ubody
assert "removed = !QFile::exists( filePath ) || QFile::remove( filePath )" in ubody
assert "return removed" in ubody
assert ubody.index("guardHistoryFile()") < ubody.index("QFile::remove") < ubody.index("unGuardHistoryFile()")
wstart=window.index("if( m_showDownloadHistory )")
wend=window.index("utility::connectQCheckBox",wstart)
wbody=window[wstart:wend]
assert "if( m_logger.clearDownloadHistory() )" in wbody
assert "m_ui->plainTextEdit->clear()" in wbody
assert "QMessageBox::warning" in wbody
print("history deletion truthfulness policy: PASS")
