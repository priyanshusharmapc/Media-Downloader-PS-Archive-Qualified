"""Regression policy for the qualified fork application updater."""
from __future__ import annotations
import argparse
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--source-root", required=True, type=Path)
args = parser.parse_args()
root = args.source_root

settings = (root / "src/settings.cpp").read_text(encoding="utf-8")
network = (root / "src/networkAccess.cpp").read_text(encoding="utf-8")

fork_api = "https://api.github.com/repos/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified/releases/latest"
upstream_release = "https://api.github.com/repos/mhogomchungu/media-downloader/releases/latest"
upstream_git = "https://api.github.com/repos/mhogomchungu/media-downloader-git/releases/latest"
asset_prefix = "/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified/releases/download/"

assert fork_api in settings, "qualified fork update API is missing"
assert upstream_release not in settings, "stable updater still targets upstream"
assert upstream_git not in settings, "git updater still targets upstream"
assert 'getOption( "ShowVersionInfoAndAutoDownloadUpdates",false )' in settings, (
    "application automatic update must default to opt-in")
assert asset_prefix in network, "downloaded update assets are not bound to the fork repository"
assert 'parsed.host().compare( "github.com",Qt::CaseInsensitive )' in network
assert 'parsed.scheme().compare( "https",Qt::CaseInsensitive )' in network
assert 'parsed.path().startsWith( expectedPrefix )' in network

print("qualified fork update-channel policy: PASS")
