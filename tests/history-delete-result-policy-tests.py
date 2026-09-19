"""Regression for MDPS-AUDIT2-098 truthful download-history clearing."""
from pathlib import Path
import argparse
p=argparse.ArgumentParser(); p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
logger=(root/"src/logger.cpp").read_text(encoding="utf-8")
window=(root/"src/logwindow.cpp").read_text(encoding="utf-8")
start=logger.index("bool Logger::clearDownloadHistory()")
end=logger.index("void Logger::reTranslateLogWindow",start)
body=logger[start:end]
assert "return QFile::remove( e )" in body
assert "QFile::remove( e ) ;" not in body.replace("return QFile::remove( e ) ;","")
wstart=window.index("if( m_showDownloadHistory )")
wend=window.index("utility::connectQCheckBox",wstart)
wbody=window[wstart:wend]
assert "if( m_logger.clearDownloadHistory() )" in wbody
assert "m_ui->plainTextEdit->clear()" in wbody
assert "QMessageBox::warning" in wbody
print("history deletion truthfulness policy: PASS")
