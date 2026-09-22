# Qualification Evidence Publication Policy

## Purpose

Release qualification must remain independently auditable after ordinary CI artifacts expire. A green workflow run by itself is not a durable evidence publication mechanism.

## Evidence classes

1. **Committed evidence**: source, tests, workflow definitions and qualification policy stored in this repository.
2. **Transient CI evidence**: Actions logs and ordinary workflow artifacts. These support development and integration but are not the long-term release record.
3. **Durable release evidence**: a commit-specific GitHub Release created only after successful `main` qualification. Compact JSON, the portable ZIP, and `current-release.json` bind identity.

## Durable release identity

The release tag is `qualification-<full-source-commit>`.

Required top-level assets are compact JSON, the portable ZIP, and `current-release.json`:

- `qualification-<full-source-commit>.json`
- `Media-Downloader-PS-Windows-Qt6-<full-source-commit>.zip`
- `current-release.json`

Do not require a second evidence zip. Inside the portable ZIP: `SHA256SUMS.txt`, `PORTABLE_MANIFEST.txt`, `RUNTIME_VERSIONS.txt`, `build-identity.json`, both executables, and bundled tools.

The compact JSON and `current-release.json` record the repository, exact commit, Actions run ID and attempt, release tag, payload filename, SHA-256 digest and retention statement.

A release for a commit is append-once from the qualification workflow's perspective. The workflow never uses an overwrite/clobber operation for existing qualification assets. On a rerun it downloads existing assets, verifies that identity matches the commit and that the ZIP hashes to the recorded SHA-256, and succeeds only when those checks match.

Historical product Release `qualification-bb949284c15a2ce1128e94d41a931fe407358e64` must not be retagged or clobbered. Every later candidate has its own immutable `qualification-<full-source-commit>` release. A release is not Gold merely because its CI qualification succeeded.

## Retention

Qualification release assets are retained until an explicit repository release deletion. They are not governed by ordinary Actions artifact expiry. Deleting a qualification release removes durable evidence and therefore invalidates any release-readiness statement that depends on it until equivalent evidence is republished and requalified.

## Historical evidence boundary

`history/RELEASE_QUALIFICATION_REPORT-4c720544.md` is a historical record. Local evidence objects from that earlier qualification environment are not present in this publication repository and must be treated as **unavailable for direct independent inspection** unless separately supplied by their custodian.

The historical record is preserved for provenance. It must not be used as current-candidate evidence. Historical `4c720544` / run `35116148963` is not current qualification.

The current candidate is always resolved from the latest commit-specific GitHub Release and its `current-release.json`. Historical `bb949284...`, verifier `77caa99...`, documentation `659f6bb...` and operational baseline `53e9c845...` remain separate provenance records. Gold status requires the repository-defined mandatory gates and is not implied by ordinary CI qualification.

## Validation

`scripts/validate-qualification-publication.py` runs in CI. It fails when the current report regresses to dangling local evidence references, when the historical availability warning disappears, or when the durable-release workflow contract is removed.
