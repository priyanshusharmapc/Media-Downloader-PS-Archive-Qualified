# Qualification Evidence Publication Policy

## Purpose

Release qualification must remain independently auditable after ordinary CI artifacts expire. A green workflow run by itself is not the durable release record.

## Final product identity

The frozen product for the completed release generation is:

- `2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- `qualification-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- qualification run `35679575106`
- payload SHA-256 `c11f21de897db6f3727fdfc8c3bdd585cd1979530864c586df824d5d8b38dd07`

Final Release Policy v2 is authoritative for release-level gate interpretation. See GitHub issue #219 and `FINAL_RELEASE_POLICY_V2.md`.

A later documentation-only/control-plane commit does not become a new product candidate merely because it is newer than C_final.

## Evidence classes

1. **Committed evidence**: source, tests, workflow definitions and policy stored in the repository.
2. **Transient CI evidence**: Actions logs and ephemeral runner state.
3. **Durable release evidence**: the commit-specific GitHub Release, compact qualification JSON, portable ZIP and `current-release.json`.
4. **Control-plane policy evidence**: owner-approved release-policy records, including issue #219 and the in-repository Policy v2 document.

## Durable release identity

Qualification releases use `qualification-<full-source-commit>`.

Required top-level assets are:

- `qualification-<full-source-commit>.json`
- `Media-Downloader-PS-Windows-Qt6-<full-source-commit>.zip`
- `current-release.json`

Inside the portable ZIP, sealed identity/runtime records include `SHA256SUMS.txt`, `PORTABLE_MANIFEST.txt`, `RUNTIME_VERSIONS.txt` and `build-identity.json`.

Qualification releases are append-once from the qualification workflow's perspective. Existing release objects must not be retagged or clobbered.

## Final gate interpretation

Ordinary CI qualification alone does not imply Gold. Gold is determined by the current controlling release policy.

Under Final Release Policy v2:

- Q25 real-provider endurance is advisory/non-blocking and currently BLOCKED;
- Q25 archive safety under incomplete discovery is PASS;
- Q26 constrained-resource qualification is advisory/non-blocking and currently BLOCKED;
- Q27 target-host media qualification is mandatory and PASS;
- all other mandatory baseline release gates passed.

The factual Q25/Q26 results are preserved. Only gate criticality changed.

## Retention

Qualification release assets are retained until explicit release deletion. Deleting a relied-upon qualification release invalidates the durable evidence chain until equivalent evidence is restored and requalified.

## Historical evidence boundary

Historical `4c720544`, `bb949284`, `77caa99`, `659f6bb` and `53e9c845` records remain provenance. They are not the final product baseline for this release generation.

Historical planning documents may contain superseded Gold-gate requirements and must be read as historical when they conflict with Final Release Policy v2.

## Resolution rule

Current product identity is resolved from the explicit frozen C_final baseline and its commit-specific release, not from the newest documentation-only commit on `main`.
