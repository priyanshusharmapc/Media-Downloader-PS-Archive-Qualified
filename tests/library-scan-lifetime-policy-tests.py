"""Regression policy for MDPS-AUDIT2-085 detached-worker owner lifetime."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root

hdr=(root/"src/library.h").read_text(encoding="utf-8")
library=(root/"src/library.cpp").read_text(encoding="utf-8")
threads=(root/"src/utils/threads.hpp").read_text(encoding="utf-8")
logger=(root/"src/logger.cpp").read_text(encoding="utf-8")
proxy=(root/"src/proxy.cpp").read_text(encoding="utf-8")
settings=(root/"src/settings.cpp").read_text(encoding="utf-8")
utility=(root/"src/utility.cpp").read_text(encoding="utf-8")
network=(root/"src/networkAccess.cpp").read_text(encoding="utf-8")
playlist=(root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")

# Library scan remains QPointer/token guarded.
assert "QPointer" in hdr
assert "std::shared_ptr< std::atomic_bool > m_scanContinue" in hdr
show=library[library.index("void library::showContents"): ]
assert "QPointer< library > m_parent" in show
assert "directoryManager::readAll( m_path,*m_continue )" in show
assert "!m_parent || !m_continue->load()" in show

# Shared helper has an opt-in QObject lifetime guard.
assert "#include <QPointer>" in threads
assert "void run( QObject * context,T bgt )" in threads
assert "QPointer< QObject > context" in threads
assert "if( context )" in threads

# Logger background work owns only immutable path state.
history=logger[logger.index("void Logger::showDownloadHistoryWindow"):logger.index("void Logger::showAllLogs")]
assert "logHistoryData( m_path )" in history
assert "utils::qthread::run( &m_logWindow" in history

# Proxy and settings no longer detach raw owners.
proxy_set=proxy[proxy.index("void proxy::set"): ]
assert "utils::qthread::run( meaw( ctx,firstTime ) )" not in proxy_set
init=settings[settings.index("void settings::init_done"):settings.index("void settings::setTabNumber")]
assert "settings& m_parent" not in init

# History append resolves Context-dependent path before detaching.
append=utility[utility.index("void utility::archiveData::addToHistory"):utility.index("QByteArray utility::archiveData::logHistoryData")]
assert "const auto historyPath = m_ctx.Engines().engineDirPaths().downloadHistoryFilePath()" in append
assert "QString m_path" in append
assert "const Context& m_ctx" not in append

# Library destructive background phases own only value state + shared tokens.
delete=library[library.index("void library::deleteEntries"):library.index("void library::setRenameUiVisible")]
assert "utils::qthread::run( this,meaw" in delete
delete_bg=delete[delete.index("bool bg()"):delete.index("void fg(")]
assert "m_parent" not in delete_bg
delete_all=library[library.index("void library::deleteAll"):library.index("void library::enableAll")]
delete_all_bg=delete_all[delete_all.index("void bg()"):delete_all.index("void fg()")]
assert "m_parent" not in delete_all_bg

# Application-update extraction worker does no owner dereference in bg(), and
# both foreground completion stages are bound to the main widget lifetime.
extract=network[network.index("void networkAccess::extractMediaDownloader"):network.index("QNetworkRequest networkAccess::networkRequest")]
bg=extract[extract.index("QString bg()"):extract.index("void fg(")]
assert "m_parent" not in bg
assert "utils::qthread::run( &m_ctx.mainWidget()" in extract
assert "QPointer< QWidget > guard" in extract
assert "if( guard )" in extract

# Playlist archiveData construction no longer runs in a detached task holding
# playlistdownloader& and engine&.
plist=playlist[playlist.index("void playlistdownloader::getList( playlistdownloader::listIterator"):playlist.index("void playlistdownloader::getList(  const QString& url")]
assert "utility::archiveData archiveData" in plist
assert "utils::qthread::run" not in plist
assert "playlistdownloader& m_parent" not in plist

print("Detached-worker owner lifetime policy: PASS")
