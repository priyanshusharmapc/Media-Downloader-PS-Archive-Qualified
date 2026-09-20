"""Regression policy for MDPS-AUDIT2-085 detached-worker owner lifetime."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root

hdr=(root/"src/library.h").read_text(encoding="utf-8")
src=(root/"src/library.cpp").read_text(encoding="utf-8")
threads=(root/"src/utils/threads.hpp").read_text(encoding="utf-8")
logger=(root/"src/logger.cpp").read_text(encoding="utf-8")
proxy=(root/"src/proxy.cpp").read_text(encoding="utf-8")
settings=(root/"src/settings.cpp").read_text(encoding="utf-8")

# Library scanning keeps cancellable state outside the Library object's lifetime
# and suppresses foreground work after QObject destruction.
assert "QPointer" in hdr
assert "std::shared_ptr< std::atomic_bool > m_scanContinue" in hdr
show=src[src.index("void library::showContents"):src.index("void library::moveUp")]
assert "QPointer< library > m_parent" in show
assert "directoryEntries bg()" in show
assert "directoryManager::readAll( m_path,*m_continue )" in show
assert "!m_parent || !m_continue->load()" in show
exit_body=src[src.index("void library::exiting"):src.index("void library::retranslateUi")]
assert "m_scanContinue.reset()" in exit_body

# Shared helper supports QObject-guarded foreground callbacks.
assert "#include <QPointer>" in threads
assert "void run( QObject * context,T bgt )" in threads
assert "QPointer< QObject > context" in threads
assert "if( context )" in threads

# Logger background work owns only a path value; its UI callback is lifetime-guarded.
history=logger[logger.index("void Logger::showDownloadHistoryWindow"):logger.index("void Logger::showAllLogs")]
assert "logHistoryData( m_path )" in history
assert "utils::qthread::run( &m_logWindow" in history
assert "*m_parent.m_ctx" not in history
assert "*m_parent->m_ctx" not in history

# System proxy lookup no longer retains Context& in a detached task.
proxy_set=proxy[proxy.index("void proxy::set"): ]
assert "QNetworkProxyFactory::systemProxyForQuery()" in proxy_set
assert "utils::qthread::run( meaw( ctx,firstTime ) )" not in proxy_set
assert "Context& m_ctx" not in proxy_set

# Settings startup worker captures immutable cleanup values only; Flatpak operations
# that require live settings state stay in the owner lifetime.
init=settings[settings.index("void settings::init_done"):settings.index("void settings::setTabNumber")]
assert "[ candidate,configPath,runningUpdated,currentExecutable ]" in init
assert "QCoreApplication::applicationFilePath()" in init
assert "isOwnedUpdateCleanupPath" in init
assert "settings& m_parent" not in init
assert "utils::qthread::run( meaw( *this ) )" not in init

print("Detached-worker owner lifetime policy: PASS")
