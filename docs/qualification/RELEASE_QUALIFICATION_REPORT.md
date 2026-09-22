# Publication Qualification Identity

## Frozen final product

The final product identity for this completed release generation is:

- Commit: `2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Qualification run: `35679575106`
- Release: `qualification-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Payload: `Media-Downloader-PS-Windows-Qt6-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2.zip`
- Payload SHA-256: `c11f21de897db6f3727fdfc8c3bdd585cd1979530864c586df824d5d8b38dd07`
- Release level: **GOLD under Final Release Policy v2**

The final policy is recorded in GitHub issue #219 and `FINAL_RELEASE_POLICY_V2.md`.

A later documentation-only/control-plane commit is not the product.

## Durable qualification evidence

The commit-specific release contains:

- `qualification-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2.json`
- `Media-Downloader-PS-Windows-Qt6-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2.zip`
- `current-release.json`

`current-release.json` binds the exact source commit, workflow run, payload filename and SHA-256. The release/tag and payload digest were verified during run `35679575106`.

The qualification workflow does not use ordinary Actions artifacts as the durable publication record.

## Final gate state

Mandatory baseline Linux/Windows/package/release-integrity gates passed.

- Q27: PASS / mandatory
- Q25 provider endurance: BLOCKED / advisory
- Q25 archive safety: PASS
- Q26 constrained-resource: BLOCKED / advisory

Q25 and Q26 are not PASS. Final Release Policy v2 makes them non-blocking for this release generation.

## Historical boundary

Earlier release and qualification records are retained for provenance. They do not supersede C_final and must not be used to infer a different final product identity.

Do not retag, clobber or rebuild the frozen final package merely to update documentation.
