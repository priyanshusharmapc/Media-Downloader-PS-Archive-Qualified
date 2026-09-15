# Media Downloader PS Archive Mode Release Qualification Report

## FINAL STATUS

**RELEASE READY**

All mandatory software, package, live, recovery, safety, GUI, sanitizer, documentation and independent-review gates passed for the candidate identified below. The earlier `MDPS-LIVE-001` defect and the Windows symlink privilege/test gap are closed. The previously accepted `MDH\Output` and first ready promotion remain immutable; this report belongs to the final ready promotion.

## Identity

- Final source commit: `261fcef529be7982b9048a39161941c1572b3044`
- Source branch: `luna-max-mdps-live-001`
- Supplied baseline: `acfd2a86d5af6ae07c4352461071e73a08b2b000`
- Authoritative CI workflow: `Archive Qt6 qualification`
- CI run: `35000478514`
- CI URL: <https://github.com/priyanshusharmapc/Media-Downloader-PS/actions/runs/35000478514>
- Exact candidate target run: `MDH\_runs\phase1-z-final-261f-rerun\target-harness.json`
- Exact candidate live run: `MDH\_runs\live\three-item-release-261f\acceptance.json`
- Final independent review session: `ses_f59c3626fffeXmUD68zZVPRta1`
- Independent review model: `openai/gpt-5.6-luna`, provider `openai`, variant `max`

## Defect Resolution

`MDPS-LIVE-001` rejected the OneDrive-style Cloud Files ancestor tag `0x9000601A`. The repaired target-host classifier uses uint32 constants and mask `0xFFFF0FFF`, accepts only the documented `0x9000n01A` family, and continues to reject links, junctions and unknown tags. The exact baseline regression is red; the repaired source and sealed candidate are green.

The Windows linked-package test now creates a real package-side symbolic link with `CreateSymbolicLinkW(link, target, 0x2)`, asserts `QFileInfo(link).isSymLink()`, and verifies that Recovery Package validation rejects it. CI enabled Developer Mode and executed this test; it was not skipped.

## Gate Results

| Gate | Result | Evidence |
|---|---|---|
| Supplied ZIP, split parts, extracted bundle and baseline portable | PASS | `Qualification-Evidence/input-verification.json`, `Qualification-Evidence/bundle-reverification.json` |
| Baseline source comparison | PASS after CRLF normalization; no substantive mismatch | `Qualification-Evidence/source-comparison.json` |
| DOCX knowledge re-verification | PASS; 24 extracted text documents and 25 DOCX files | `Qualification-Evidence/docx-reverification.json` |
| MDPS-LIVE-001 regression | PASS; baseline red, repaired source/candidate green | `Qualification-Evidence/mdps-live-001-regression.json` |
| Linux build, CTest, ASan and UBSan | PASS | `Qualification-Evidence/ci-linux-LastTest.log`, `Qualification-Evidence/ci-linux-sanitized-LastTest.log` |
| Windows Qt6 build and package | PASS | `Qualification-Evidence/ci-run.json`, `Qualification-Evidence/ci-windows-LastTest.log` |
| Windows CTest | PASS; 19 tests, 0 failures, 0 skips | `Qualification-Evidence/ci-windows-ctest-junit.xml`, `Qualification-Evidence/ci-windows-LastTest.log` |
| Real Windows linked-package symbolic-link test | PASS | `Qualification-Evidence/ci-windows-ctest-junit.xml` |
| Exact candidate package seal | PASS; 116/116 entries match, no unsealed files | `Qualification-Evidence/ci-candidate-seal.json` |
| Exact candidate target-host harness on Z | PASS | `Qualification-Evidence/target-harness-binding.json` |
| Exact candidate three-item live test | PASS; video/audio, FFprobe, restart and idempotency | `Qualification-Evidence/three-item-binding.json` |
| Exact candidate unavailable-source test | PASS; failed identity retained, no media published | `Qualification-Evidence/unavailable-binding.json` |
| Crash, transaction, locking, concurrency and corruption tests | PASS | CI CTest and integration evidence |
| Filesystem, junction, linked package and long-path safety | PASS | `Qualification-Evidence/filesystem-safety.json`, CI CTest evidence |
| Recovery Packages | PASS | `Qualification-Evidence/recovery-packages.json` |
| Tamper and unsealed-file rejection | PASS | `Qualification-Evidence/package-tamper.json` |
| Backup, restore and relocation | PASS | `Qualification-Evidence/backup-relocation.json` |
| 1,000-item scale and 30-cycle soak | PASS | `Qualification-Evidence/scale.json`, `Qualification-Evidence/soak.json` |
| GUI startup smoke | PASS | `Qualification-Evidence/gui-smoke.json`, CI evidence |
| Documentation and publication-source QA | PASS | `Qualification-Evidence/documentation-qa.json`, `Qualification-Evidence/source-cleanliness.json` |
| Independent Luna Max review | PASS; no remaining release blocker | `Qualification-Evidence/model-routing.json` |

## Package Hashes

- Portable ZIP: `Media-Downloader-PS-Portable-261fcef5.zip` SHA-256 `07DEAD80E265FED9252FF6D38660E84430D20627A154A9C80A048D59629D1D8F`
- Curated source ZIP: `Media-Downloader-PS-GitHub-Source-261fcef5.zip` SHA-256 `04893D2851AA5C48C97AD57728A61AFAB3F92204B84B5AC9F9B4BB215405B042`
- Portable `archive-cli.exe`: `8388abd9f129fca9f45e77cf60d7e73ebf596b40816016d0fab3cdeab93ad0dc`
- Portable `media-downloader.exe`: `cb890dd5542464d2b476b7844a1a169d1059e6d187cbb9702cb6bba5f1850516`
- Portable `archive-local-harness.ps1`: `806d828fcb87bbb1265b6b2f34fa704dd74aceade6835bb781b9f00ff807c1f2`
- Portable internal manifest: 116 files, all matched.

## Documentation

The curated documentation covers installation, GUI and CLI operation, Archive Root state and history, media verification, transactions, locking, Recovery Packages, backup/restore, relocation, filesystem security, troubleshooting, limitations, testing, qualification, release, source mapping, licensing and valid examples. It contains no forbidden local paths or credential patterns.

## Final Output

The final release-ready promotion is a new immutable directory:

```text
Output-Release-Ready-Final/
  Media-Downloader-PS-Portable/
  Media-Downloader-PS-Portable-261fcef5.zip
  GitHub-Source/
  Media-Downloader-PS-GitHub-Source-261fcef5.zip
  Documentation/
  Qualification-Evidence/
  RELEASE_NOTES.md
  RELEASE_QUALIFICATION_REPORT.md
  SOURCE_COMMIT.txt
  SHA256SUMS-OUTPUT.txt
```

The outer manifest excludes itself to avoid a self-referential hash. No file in the release-ready Output may be modified after final acceptance.
