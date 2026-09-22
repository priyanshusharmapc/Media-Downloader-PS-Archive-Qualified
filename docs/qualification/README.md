# Media Downloader PS Archive Mode

## Final release identity

Frozen product baseline:

- Commit: `2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Qualification run: `35679575106`
- Release: `qualification-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Portable ZIP SHA-256: `c11f21de897db6f3727fdfc8c3bdd585cd1979530864c586df824d5d8b38dd07`
- Release level: **GOLD under Final Release Policy v2**

Final Release Policy v2 is documented in GitHub issue #219 and `FINAL_RELEASE_POLICY_V2.md`.

A later documentation-only/control-plane commit on `main` does not supersede this product identity.

## Qualification status

Mandatory gates passed, including Linux, Windows, sanitizer, portable package, packaged runtime, release identity verification, and Q27 target-host acceptance.

- Q25 provider endurance: BLOCKED / advisory
- Q25 archive safety: PASS
- Q26 constrained-resource qualification: BLOCKED / advisory
- Q27 target-host qualification: PASS / mandatory

Q25 and Q26 are not PASS.

Audit2 is formally closed at tail `MDPS-AUDIT2-251`.

## Product use

The portable application directory is sealed and should not be modified. Archive Root is separate writable operator data. The GUI initializes Archive Root automatically; CLI or scripted use may run `archive-cli.exe preflight <archive-root>` before the first operation.

Security includes traversal, symlink, junction and reparse-point rejection.

## Qualification documents

- `FINAL_RELEASE_POLICY_V2.md`
- `PROJECT_STATUS.json`
- `TESTING_QUALIFICATION.md`
- `RELEASE_QUALIFICATION_REPORT.md`
- `RELEASE_NOTES.md`
- `EVIDENCE_POLICY.md`
- `AUDIT2_INTEGRATION_STATUS.md`
