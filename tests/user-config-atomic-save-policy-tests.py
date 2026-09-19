"""Regression policy for MDPS-AUDIT2-052 atomic user configuration persistence."""
from __future__ import annotations
import argparse
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument("--source-root",required=True,type=Path)
root=p.parse_args().source_root
playlist=(root/"src/playlistdownloader.cpp").read_text(encoding="utf-8")
configure=(root/"src/configure.cpp").read_text(encoding="utf-8")
playlist_h=(root/"src/playlistdownloader.h").read_text(encoding="utf-8")
configure_h=(root/"src/configure.h").read_text(encoding="utf-8")

assert "bool save() ;" in playlist_h
assert configure_h.count("bool save() ;") >= 2
ps=playlist[playlist.index("bool playlistdownloader::subscription::save()"):]
assert "QSaveFile f( m_path )" in ps
assert "f.write( data ) != data.size()" in ps
assert "f.cancelWriting()" in ps
assert "return f.commit()" in ps
assert "QMessageBox::warning" in playlist

for sig,next_sig in [
    ("bool configure::presetOptions::save()","void configure::presetOptions::clear()"),
    ("bool configure::downloadDefaultOptions::save()","void configure::setVisibilityEditConfigFeature")
]:
    start=configure.index(sig)
    end=configure.index(next_sig,start)
    body=configure[start:end]
    assert "QSaveFile f( m_path )" in body
    assert "f.write( data ) != data.size()" in body
    assert "f.cancelWriting()" in body
    assert "return f.commit()" in body
    assert "QIODevice::Truncate" not in body

assert "const auto defaultOptionsSaved = m_downloadDefaultOptions.save()" in configure
assert "const auto engineDefaultsSaved = m_downloadEngineDefaultOptions.save()" in configure
assert "if( !m_presetOptions.save() )" in configure
assert configure.count("QMessageBox::warning") >= 2
print("atomic user configuration persistence policy: PASS")

# A failed save must not leave the in-memory/UI model ahead of durable state.
assert "const auto previous = m_array" in playlist
assert "m_array = previous" in playlist
assert "this->setVisible( true )" in playlist
assert "visible subscription list were restored" in playlist
