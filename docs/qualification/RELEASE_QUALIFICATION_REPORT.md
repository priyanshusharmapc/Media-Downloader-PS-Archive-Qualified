# Media Downloader PS Archive Mode Release Qualification Report

## FINAL STATUS

**NOT RELEASE READY**

The tested software and sealed CI candidate pass the executed product, package, live, recovery, safety, GUI, sanitizer and documentation checks. Release is withheld because two mandatory gates remain blocked and cannot be represented as PASS:

- `LUNA-MAX-DELEGATED-REVIEW`: Agent Manager resolved both requested dispatches to `GPT-5.6 Luna`, provider `openai`, variant `max`, but the overview exposed no session ID or runtime metadata. No delegated worker result was used as evidence.
- `WINDOWS-SYMLINK-PRIVILEGE`: the exact Windows symlink test returned its documented host-dependent skip status. Direct Z-path junction/root/state/package rejection passed, and Linux sanitized coverage passed, but the missing Windows symlink privilege prevents closing this exact gate.

## Identity

- Final source commit: `ad7943b93b0458ff2764432ae67419104c43fff8`
- Source branch: `luna-max-mdps-live-001`
- Known supplied baseline: `acfd2a86d5af6ae07c4352461071e73a08b2b000`
- Authoritative CI workflow: `Archive Qt6 qualification`
- CI run: `34981539523`
- CI URL: <https://github.com/priyanshusharmapc/Media-Downloader-PS/actions/runs/34981539523>
- Portable candidate source identity: commit `ad7943b93b0458ff2764432ae67419104c43fff8`, CI run `34981539523`

## Repair

`MDPS-LIVE-001` was a real target-host harness defect. The old classifier rejected the OneDrive-style Cloud Files tag `0x9000601A`; its bit mask ignored the wrong nibble and its PowerShell comparison used signed literals. The repaired script uses uint32 constants with mask `0xFFFF0FFF`, accepts only the documented `0x9000n01A` family, and continues to reject symbolic links, junctions and unknown reparse tags.

The exact baseline regression exits nonzero. The repaired repository script and the repaired sealed candidate both pass. The repair was committed, pushed, built by CI and exercised by the exact candidate target harness.

## Gate Results

| Gate | Result | Evidence |
|---|---|---|
| Supplied ZIP, split parts and extracted bundle integrity | PASS | `Qualification-Evidence/input-verification.json`, `Qualification-Evidence/bundle-reverification.json` |
| Baseline portable manifest | PASS | `Qualification-Evidence/bundle-reverification.json` |
| Baseline source comparison | PASS after CRLF normalization; no substantive mismatch | `Qualification-Evidence/source-comparison.json` |
| DOCX knowledge re-verification | PASS; 24 extracted text documents and 25 supplied DOCX files hashed | `Qualification-Evidence/docx-reverification.json` |
| Source/parser regression for `MDPS-LIVE-001` | PASS; baseline red, repaired source and candidate green | `Qualification-Evidence/mdps-live-001-regression.json` |
| Linux build, CTest, ASan and UBSan | PASS in CI | `Qualification-Evidence/ci-linux-LastTest.log`, `Qualification-Evidence/ci-linux-sanitized-LastTest.log`, `Qualification-Evidence/ci-linux-ctest-junit.xml`, `Qualification-Evidence/ci-linux-sanitized-ctest-junit.xml` |
| Windows Qt6 build and package creation | PASS in CI | `Qualification-Evidence/ci-run.json`, `Qualification-Evidence/ci-windows-LastTest.log` |
| Windows CTest | 18 PASS, 1 explicit symlink-privilege skip | `Qualification-Evidence/ci-windows-LastTest.log`, `Qualification-Evidence/ci-windows-ctest-junit.xml` |
| Exact CI candidate manifest | PASS; 121/121 entries match, no unsealed file | `Qualification-Evidence/ci-candidate-seal.json` |
| Exact repaired target-host harness on relocated Z path | PASS | `Qualification-Evidence/target-harness.json` |
| Live playlist runtime probe | PASS; exactly three currently accessible items selected | `Qualification-Evidence/live-selection.json` |
| Three-item video/audio, FFprobe, restart and idempotency | PASS | `Qualification-Evidence/three-item-acceptance.json`, `Qualification-Evidence/media-verification.json` |
| Unavailable/private/deleted behavior | PASS; nine historical candidates unavailable, failed identity retained, partial scan non-destructive | `Qualification-Evidence/historical-unavailable.json`, `Qualification-Evidence/unavailable-acceptance.json`, CI CTest logs |
| Crash recovery, transactions, locking, concurrency and corruption | PASS in deterministic integration suite | CI CTest logs and `Qualification-Evidence/ci-windows-ctest-junit.xml` |
| Z filesystem safety and reparse rejection | PASS for fresh roots, junctions, linked state, linked package and long path | `Qualification-Evidence/filesystem-safety.json` |
| Windows symlink-specific privilege gate | BLOCKED; documented CTest skip | `Qualification-Evidence/ci-windows-LastTest.log` |
| Recovery Package validation, acceptance and rejection | PASS | `Qualification-Evidence/recovery-packages.json` |
| Portable tamper and unsealed-file rejection | PASS | `Qualification-Evidence/package-tamper.json` |
| Backup, restore and root relocation | PASS | `Qualification-Evidence/backup-relocation.json` |
| 1,000-item scale and 30-cycle soak | PASS | `Qualification-Evidence/scale.json`, `Qualification-Evidence/soak.json` |
| GUI startup smoke | PASS locally and in CI; no fatal output | `Qualification-Evidence/gui-smoke.json`, `Qualification-Evidence/ci-run.json` |
| Documentation completeness and leakage QA | PASS; seven curated documents, required topics present, no forbidden local paths/secrets | `Qualification-Evidence/documentation-qa.json` |
| Publication source cleanliness | PASS; internal qualification logs and generated IDE metadata excluded | `Qualification-Evidence/source-cleanliness.json` |
| Luna Max delegated-worker routing | BLOCKED; no effective Agent Manager session metadata | `Qualification-Evidence/model-routing.json` |

## Package Hashes

- Portable ZIP: `Media-Downloader-PS-Portable-ad7943b9.zip` SHA-256 `D08AE88A5CF1CC8CB450B3B2B141B26D157F337B7452875C3D0A205111DB44E4`
- Curated GitHub source ZIP: `Media-Downloader-PS-GitHub-Source-ad7943b9.zip` SHA-256 `80615BED1F7B7220DBDBF973BCFF9EA460754F1DBA52233866EB7445656BB17B`
- Portable `archive-cli.exe`: `481abf073856253d81eda96c54c423ffa1be3eccd7d094abc832d249f19defd6`
- Portable `media-downloader.exe`: `21c6be6f35020eddbcc9fd4d7f2653f377634b266edefa01379d056134d96c16`
- Portable `archive-local-harness.ps1`: `806d828fcb87bbb1265b6b2f34fa704dd74aceade6835bb781b9f00ff807c1f2`
- Portable internal manifest: 121 files, all matched in `Qualification-Evidence/ci-candidate-seal.json`

## Defects

- `MDPS-LIVE-001`: resolved by the committed Cloud Files classifier repair and regression. No P0, P1 or P2 product defect remains in the executed evidence.
- Local full-GUI CTest through the temporary drive alias showed Windows integration cleanup errors, while the direct Z archive suite and authoritative CI passed. This is retained as local toolchain/alias evidence and is not silently counted as a product PASS.
- No supplied original, supplied qualified portable, DOCX, bundle ZIP or final candidate was modified.

## Documentation

The curated `Documentation/` directory contains installation and user/GUI/CLI operation, state and history, media verification, transaction and lock behavior, Recovery Packages, backup/restore, relocation, security, troubleshooting, limitations, build/test/qualification/release controls, source map, links, licensing and valid examples. It contains no local/private workspace path or credential pattern.

## Final Output Layout

The final Output directory is assembled only after this report and all staged files are complete:

```text
Output/
  Media-Downloader-PS-Portable/
  Media-Downloader-PS-Portable-ad7943b9.zip
  GitHub-Source/
  Media-Downloader-PS-GitHub-Source-ad7943b9.zip
  Documentation/
  Qualification-Evidence/
  RELEASE_NOTES.md
  RELEASE_QUALIFICATION_REPORT.md
  SOURCE_COMMIT.txt
  SHA256SUMS-OUTPUT.txt
```

`SHA256SUMS-OUTPUT.txt` is the outer manifest and excludes itself to avoid a self-referential digest. No file under `Output` may be changed after exact-byte acceptance begins.
