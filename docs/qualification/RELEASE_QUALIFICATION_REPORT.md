# Publication qualification identity

This document describes how a **current** candidate is qualified and how its evidence is published durably. It does not reuse the historical release-ready claim as evidence for a newer commit.

## Current candidate identity

The qualified product commit is `bb949284c15a2ce1128e94d41a931fe407358e64`. GitHub Release `qualification-bb949284c15a2ce1128e94d41a931fe407358e64` holds the portable ZIP (`payload_sha256=8704235825ab2b5f9a3c6fe5eadd052ec08402214f7225d3e040b94f6b0e1cfd`) from run `35613369266`. Commit `77caa99e421ce6d9476d02c3d34c927d963b2528` is a verifier-only follow-up (`GH_REPO` on the ubuntu publisher). Sibling Release `qualification-77caa99e421ce6d9476d02c3d34c927d963b2528` is not the gold product. Gold 4 (endurance and low-memory) is **paused**: `GOLD-4-PAUSED-LOWMEM` (no cmake/ninja/Qt on the L16 host) and `GOLD-4-PAUSED-ENDURANCE` (no operator PlaylistUrl/VideoUrl). This is not a complete gold-state release.

Do not retag `qualification-bb949284c15a2ce1128e94d41a931fe407358e64`. A later docs commit on `main` is not the product.

For every candidate, GitHub Actions generates `current-release.json` beside the qualified Windows payload. That generated record binds:

- repository
- Git ref
- exact source commit
- GitHub Actions run ID and attempt
- artifact name
- payload ZIP name
- SHA-256 of that exact payload ZIP

The workflow validates those fields against GitHub's runtime identity before upload.

## Durable qualification evidence

Ordinary GitHub Actions artifacts are useful working evidence but have finite retention and are **not** the durable publication record.

A successful qualification push to `main` must also complete the `publish-qualification-evidence` job. That job verifies a commit-specific GitHub Release tagged:

`qualification-<full-source-commit>`

The release contains these top-level assets (no second evidence zip):

- compact JSON `qualification-<full-source-commit>.json` — repository, commit, workflow run/attempt, linux/windows success flags
- portable ZIP `Media-Downloader-PS-Windows-Qt6-<full-source-commit>.zip`
- `current-release.json` — payload SHA-256 and run identity

The Windows job is the single `gh release create` writer. The ubuntu `publish-qualification-evidence` job verifies tag peel, required assets, and `payload_sha256`. The publisher is fail-closed. If assets for the same commit already exist, a rerun verifies the existing identity and ZIP digest and does not overwrite them. A differing existing object fails publication.

A candidate is not durably qualified merely because PR CI or an integration-branch workflow is green. Durable publication requires the successful commit-specific release job above.

See `EVIDENCE_POLICY.md` for the publication/retention contract.

## Historical record

The pre-publication qualification record is preserved at `history/RELEASE_QUALIFICATION_REPORT-4c720544.md`. Historical `4c720544` / run `35116148963` is not current qualification. The historical record is not evidence for a current candidate.

No current qualification claim may cite unavailable historical local evidence paths as if they were directly inspectable publication evidence.
