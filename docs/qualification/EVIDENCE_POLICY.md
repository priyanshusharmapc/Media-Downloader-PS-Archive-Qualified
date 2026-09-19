# Qualification Evidence Publication Policy

## Purpose

Release qualification must remain independently auditable after ordinary CI artifacts expire. A green workflow run by itself is not a durable evidence publication mechanism.

## Evidence classes

1. **Committed evidence**: source, tests, workflow definitions and qualification policy stored in this repository.
2. **Transient CI evidence**: Actions logs and ordinary workflow artifacts. These support development and integration but are not the long-term release record.
3. **Durable release evidence**: a commit-specific GitHub Release created only after successful `main` qualification. Its evidence bundle is bound by SHA-256 in a separate manifest.

## Durable release identity

The release tag is `qualification-<full-source-commit>`.

Required assets are:

- `qualification-evidence-<full-source-commit>.zip`
- `qualification-evidence-manifest-<full-source-commit>.json`

The manifest records the repository, exact commit, Actions run ID and attempt, release tag, bundle filename, SHA-256 digest and retention statement.

A release for a commit is append-once from the qualification workflow's perspective. The workflow never uses an overwrite/clobber operation for existing qualification assets. On a rerun it downloads existing assets, verifies that the manifest is byte-identical to the newly derived identity and that the bundle hashes to the recorded SHA-256, and succeeds only when those checks match.

## Retention

Qualification release assets are retained until an explicit repository release deletion. They are not governed by ordinary Actions artifact expiry. Deleting a qualification release removes durable evidence and therefore invalidates any release-readiness statement that depends on it until equivalent evidence is republished and requalified.

## Historical evidence boundary

`history/RELEASE_QUALIFICATION_REPORT-4c720544.md` is a historical record. Its `Qualification-Evidence/...` paths referred to local evidence from the earlier qualification environment. Those objects are not present in this publication repository and must be treated as **unavailable for direct independent inspection** unless separately supplied by their custodian.

The historical record is preserved for provenance. It must not be used as current-candidate evidence.

## Validation

`scripts/validate-qualification-publication.py` runs in CI. It fails when the current report regresses to dangling local evidence references, when the historical availability warning disappears, or when the durable-release workflow contract is removed.
