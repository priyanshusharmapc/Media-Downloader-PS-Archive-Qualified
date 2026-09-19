"""Fail closed if publication qualification identity regresses to stale hardcoded evidence."""
from __future__ import annotations
import argparse
import re
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--source-root", required=True, type=Path)
args = parser.parse_args()
root = args.source_root

report = (root / "docs/qualification/RELEASE_QUALIFICATION_REPORT.md").read_text(encoding="utf-8")
source_commit = (root / "docs/qualification/SOURCE_COMMIT.txt").read_text(encoding="utf-8")
workflow = (root / ".github/workflows/archive-qt6.yml").read_text(encoding="utf-8")

assert "4c72054465697b899ede0555e48aff8a7c91c225" not in report, "current report still claims historical commit"
assert "priyanshusharmapc/Media-Downloader-PS`" not in report, "current report still claims historical repository"
assert source_commit.startswith("GENERATED_BY_QUALIFICATION_WORKFLOW"), "SOURCE_COMMIT.txt must not masquerade as a static current SHA"
assert not re.search(r"\b[0-9a-f]{40}\b", source_commit), "SOURCE_COMMIT.txt contains a stale hardcoded SHA"

required = [
    "$env:GITHUB_REPOSITORY", "$env:GITHUB_SHA", "$env:GITHUB_RUN_ID",
    "$env:GITHUB_RUN_ATTEMPT", "$env:GITHUB_REF", "payload_sha256",
    "current-release.json", "Media-Downloader-PS-Windows-Qt6-"
]
for token in required:
    assert token in workflow, f"workflow does not bind generated release identity to {token}"

history = root / "docs/qualification/history/RELEASE_QUALIFICATION_REPORT-4c720544.md"
history_source = root / "docs/qualification/history/SOURCE_COMMIT-4c720544.txt"
assert history.exists() and history_source.exists(), "historical qualification evidence was not preserved"
print("publication release identity policy: PASS")
