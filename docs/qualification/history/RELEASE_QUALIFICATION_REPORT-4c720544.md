# Historical Qualification Record: Media Downloader PS Archive Mode

## Historical status at the recorded source

**HISTORICAL RELEASE-READY CLAIM, NOT CURRENT PUBLICATION STATUS**

The final candidate passed the repaired implementation, regression, Windows/Linux, sanitizer, package, target-host, live-media, recovery, filesystem, GUI-settings, low-memory and endurance gates. The final source/package identity is unambiguous below. No P0, P1 or release-blocking P2 product defect remains.

## Historical evidence availability

The `Qualification-Evidence/...` paths below are references to the original private/local qualification workspace. Those evidence objects are **not included in this publication repository and are unavailable for direct independent inspection from this repository**. They are preserved here only to document what the historical qualification record claimed to have inspected. This historical record is not current-candidate qualification evidence.

## Identity

- Repository: `priyanshusharmapc/Media-Downloader-PS`
- Branch: `audit-remediation-20260916`
- Final source commit: `4c72054465697b899ede0555e48aff8a7c91c225`
- Final authoritative CI run: `35116148963`
- CI URL: <https://github.com/priyanshusharmapc/Media-Downloader-PS/actions/runs/35116148963>
- Artifact: `Media-Downloader-PS-Windows-Qt6-4c72054465697b899ede0555e48aff8a7c91c225`
- Build identity and package: `Qualification-Evidence/ci-run.json`, `Qualification-Evidence/final-package-seal.json`

## Defect Resolution

- `MDPS-AUDIT-001`: Full FFmpeg stream consumption with `-xerror` now follows profile metadata validation. Real fast-start truncation and corrupt-media regressions fail verification; sync repairs damaged canonical media.
- `MDPS-AUDIT-002`: Requested Archive Root and existing parent components are checked before canonical resolution. Root and parent symlink/junction regressions pass.
- `MDPS-AUDIT-003`: C++ Windows reparse policy inspects tags, permits only the documented Cloud Files family, and rejects links, junctions, name-surrogate and unknown unsafe tags.
- `MDPS-AUDIT-004`: GUI settings use per-user Qt application configuration. The guarded packaged settings smoke confirms no package-local unsealed file is created and the sealed manifest remains unchanged.
- `MDPS-AUDIT-005`: Inherited upstream publisher workflows and the temporary remediation workflow are removed. Only the repository-owned Archive Qt6 workflow remains active.
- `MDPS-AUDIT-006`: Final package is generated from exactly source commit `4c720544` by CI run `35116148963`; no older package is used as final identity.
- `MDPS-AUDIT-007`: Identity-bound endurance ran 60 minutes and 1,102 iterations with zero failures, no media hash/set changes, zero temporary residue and no transaction journal.
- `MDPS-AUDIT-008`: Unresolved placeholder base identity excludes mutable playlist position; duplicate occurrence matching preserves identities across reorder/insertion.
- `MDPS-AUDIT-009`: Ordinary and superscript Windows reserved device-name variants are rejected.
- `MDPS-AUDIT-010`: `verify-item` documentation now distinguishes profile/full-decode integrity from separate SHA-256 idempotency evidence.
- `MDPS-AUDIT-011`: Executable source modes are preserved in the qualified source tree.

## Gate Results

| Gate | Result | Evidence |
|---|---|---|
| Interrupted-state recovery and patch preservation | PASS | `Qualification-Evidence/INTERRUPTED-RUN-RECOVERY.md`, `Qualification-Evidence/interrupted-local-work.patch` |
| Supplied input/baseline integrity | PASS | `Qualification-Evidence/input-verification.json`, `Qualification-Evidence/bundle-reverification.json` |
| Source completeness and publication cleanliness | PASS | `Qualification-Evidence/source-cleanliness.json`, `Qualification-Evidence/source-comparison.json` |
| DOCX knowledge and documentation QA | PASS | `Qualification-Evidence/docx-reverification.json`, `Qualification-Evidence/documentation-qa.json` |
| Media truncation/corruption regression | PASS | CI CTest, integration suite and `Qualification-Evidence/mdps-live-001-regression.json` |
| Core root/link/reparse safety | PASS | CI Windows CTest, `Qualification-Evidence/ci-windows` |
| GUI settings/package immutability | PASS | `Qualification-Evidence/ci-windows/gui-settings-test/results.json`, `Qualification-Evidence/gui-smoke.json` |
| Linux normal and sanitized tests | PASS | `Qualification-Evidence/ci-linux` |
| ASan and UBSan | PASS with no diagnostics | `Qualification-Evidence/ci-linux/build-sanitized/Testing/Temporary/LastTest.log` |
| Windows CTest and integration | PASS; 29/29 CTest, 23 integration cases | `Qualification-Evidence/ci-windows/build/Testing/Temporary/LastTest.log`, `Qualification-Evidence/ci-windows/build/Testing/Temporary/ctest-junit.xml` |
| Low-memory local build/CTest | PASS; `--parallel 1`, five local fixture skips explicitly covered by authoritative CI | `Qualification-Evidence/low-memory-4c720.json` |
| Exact package seal | PASS; 116/116 manifest entries, zero mismatch/unsealed files | `Qualification-Evidence/final-package-seal.json` |
| Exact target-host acceptance | PASS | `Qualification-Evidence/target-harness.json` |
| Three-item live verification | PASS; three real items harvested and verified on the code-equivalent candidate; only fix-list documentation changed afterward | `Qualification-Evidence/live-three-item-binding.json` |
| Unavailable/private/deleted behavior | PASS | `Qualification-Evidence/unavailable-acceptance.json`, CI integration evidence |
| Recovery Packages, transactions, crash recovery, locking, concurrency and corruption | PASS | `Qualification-Evidence/recovery-packages.json`, CI integration evidence |
| Backup/restore and relocation | PASS | `Qualification-Evidence/backup-relocation.json` |
| Tamper rejection | PASS | `Qualification-Evidence/package-tamper.json` |
| Scale and soak regressions | PASS | `Qualification-Evidence/scale.json`, `Qualification-Evidence/soak.json` |
| 60-minute endurance | PASS; 1,102 iterations, zero failures; code-equivalent binding proves no executable/test-code delta afterward | `Qualification-Evidence/endurance.json`, `Qualification-Evidence/endurance-binding.json` |
| GUI startup and bundled runtime smoke | PASS | `Qualification-Evidence/gui-smoke.json`, CI Windows job evidence |
| Independent Luna Max final review | PASS; local fixture skips correctly classified as CI-covered | `Qualification-Evidence/final-reviews.json` |
| Independent Grok 4.6 XAI high critique | PASS | `Qualification-Evidence/final-reviews.json` |

## Package Hashes

- Portable ZIP: `Media-Downloader-PS-Portable-4c720544.zip` SHA-256 `4EA533866FD06223E373B4C9BD5632E76E12A75B857D0CBB544B7860AD88A3B7`
- Curated source ZIP: `Media-Downloader-PS-GitHub-Source-4c720544.zip` SHA-256 `7DF2224ECA73C88BA34EA7EF8219603DB6A6E734A85A8AB743B13D4AE7CD0E6F`
- `archive-cli.exe`: `7241ab79d086d59cffb0b54a444dc0681900bcc7b55d253516bc91a5acdda4c7`
- `media-downloader.exe`: `1c8588a774432e18f0e57ba8961ed3448d46d0989f3e02ec5008845c4372a5bb`
- `archive-local-harness.ps1`: `806d828fcb87bbb1265b6b2f34fa704dd74aceade6835bb781b9f00ff807c1f2`
- Portable internal manifest: 116 entries, all matched.

## Endurance

The run began after the code/test implementation was stable. The exact sealed CI package was independently manifest-validated as `BasePackageRoot`; the fake downloader was an explicitly recorded test overlay, so the sealed application package was not modified. The run lasted from `2026-09-16T13:29:39.8834421+05:30` to `2026-09-16T14:29:41.0652845+05:30`, completed 1,102 iterations, observed no failures or media changes, and left zero temporary entries and no transaction journal. The final source changes after that run were documentation-only, proven by `Qualification-Evidence/endurance-binding.json`; no executable or test-code file changed.

## Documentation Boundary

The final documentation covers architecture, state/history, media integrity, GUI/CLI operation, settings location, transactions, locking, Recovery Packages, filesystem/reparse safety, backup/restore, relocation, security, troubleshooting, limitations, testing, CI, qualification, release and licensing. Native file-dialog click automation is not used in offscreen CI; the packaged executable settings path and package seal are tested through the guarded qualification smoke.

## Final Output

```text
Output-Audit-Remediated-Final/
  Media-Downloader-PS-Portable/
  Media-Downloader-PS-Portable-4c720544.zip
  GitHub-Source/
  Media-Downloader-PS-GitHub-Source-4c720544.zip
  Documentation/
  Qualification-Evidence/
  RELEASE_NOTES.md
  RELEASE_QUALIFICATION_REPORT.md
  SOURCE_COMMIT.txt
  SHA256SUMS-OUTPUT.txt
```

The output manifest excludes itself to avoid self-reference. No final Output file may be changed after acceptance.
