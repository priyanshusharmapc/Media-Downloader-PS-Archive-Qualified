# Publication qualification identity

This document describes how a **current** candidate is qualified and how its evidence is published durably. It does not reuse the historical release-ready claim as evidence for a newer commit.

## Current candidate identity

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

A successful qualification push to `main` must also complete the `publish-qualification-evidence` job. That job publishes a commit-specific GitHub Release tagged:

`qualification-<full-source-commit>`

The release contains:

- `qualification-evidence-<full-source-commit>.zip` - the source, Linux/Windows test evidence, qualified package material and generated release identity downloaded from the same workflow run.
- `qualification-evidence-manifest-<full-source-commit>.json` - repository, commit, workflow run/attempt, release tag, bundle filename, bundle SHA-256 and retention semantics.

The publisher is fail-closed. If assets for the same commit already exist, a rerun verifies the existing manifest and bundle digest and does not overwrite them. A differing existing object fails publication.

A candidate is not durably qualified merely because PR CI or an integration-branch workflow is green. Durable publication requires the successful commit-specific release job above.

See `EVIDENCE_POLICY.md` for the publication/retention contract.

## Historical record

The pre-publication qualification record is preserved at `history/RELEASE_QUALIFICATION_REPORT-4c720544.md`. Its original local qualification-evidence paths describe evidence that was available to that historical qualification process but is not included in this publication repository. The historical record is not evidence for a current candidate.

No current qualification claim may cite those unavailable historical paths as if they were directly inspectable publication evidence.
