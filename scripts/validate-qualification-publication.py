#!/usr/bin/env python3
"""Fail closed when qualification documentation regresses to dangling evidence."""
from __future__ import annotations

import argparse
from pathlib import Path


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit("qualification-publication validation failed: " + message)


parser = argparse.ArgumentParser()
parser.add_argument("--source-root", type=Path, required=True)
root = parser.parse_args().source_root.resolve()

qualification = root / "docs" / "qualification"
current = (qualification / "RELEASE_QUALIFICATION_REPORT.md").read_text(encoding="utf-8")
source_map = (qualification / "SOURCE_MAP.md").read_text(encoding="utf-8")
policy = (qualification / "EVIDENCE_POLICY.md").read_text(encoding="utf-8")
historical = (qualification / "history" / "RELEASE_QUALIFICATION_REPORT-4c720544.md").read_text(encoding="utf-8")
workflow = (root / ".github" / "workflows" / "archive-qt6.yml").read_text(encoding="utf-8")

require("Qualification-Evidence/" not in current,
        "current report cites unpublished Qualification-Evidence paths")
require("publish-qualification-evidence" in current,
        "current report does not require durable evidence publication")
require("qualification-<full-source-commit>" in current,
        "current report lacks commit-specific release identity")
require((qualification / "history" / "RELEASE_QUALIFICATION_REPORT-4c720544.md").is_file(),
        "current report historical reference is dangling")
require((qualification / "EVIDENCE_POLICY.md").is_file(),
        "evidence policy is missing")
require("unavailable for direct independent inspection" in historical,
        "historical local evidence is not explicitly marked unavailable")
require("not current-candidate qualification evidence" in historical,
        "historical record can be mistaken for current evidence")
require("qualified implementation source remains bound to commit" not in source_map,
        "source map still presents the historical commit as current qualification")
require("publish-qualification-evidence:" in workflow,
        "durable evidence publication job is missing")
require("contents: write" in workflow,
        "durable publication job cannot create a release")
require("qualification-evidence-manifest-" in workflow,
        "workflow does not create a digest-bearing evidence manifest")
require("gh release upload" in workflow,
        "workflow does not publish evidence as release assets")
require("--clobber" not in workflow,
        "qualification assets are overwriteable")
require("qualification evidence bundle SHA-256 mismatch" in workflow,
        "rerun does not verify the existing durable bundle digest")
require("ordinary Actions artifact expiry" in policy,
        "retention policy does not distinguish transient artifacts")

print("qualification publication policy: PASS")
