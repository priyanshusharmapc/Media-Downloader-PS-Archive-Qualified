# Defect Ledger

Severity: P0 destructive/critical security; P1 historical loss/false completion/wrong canonical data/recovery corruption; P2 major workflow/crash; P3 material recoverable normal-workflow defect; P4 minor. P0-P2 and normal-workflow P3 block release.

## MDPS-LIVE-001: Harness Rejects Non-Redirecting OneDrive Cloud Ancestors
- Severity: P2, target-host acceptance blocked in the baseline; resolved on the repaired branch.
- Affected commit: acfd2a86d5af6ae07c4352461071e73a08b2b000.
- Reproduction: invoke sealed original-copy harness with empty external Archive Root under required MDH location, using current playlist item 0fvbzgnVO2Y.
- Evidence: `_runs/phase1/first-harness.json`, stdout/stderr. Exit 1 before binaries executed: linked path refused at OneDrive Desktop.
- Root cause: Safe-Child rejects every FileAttributes.ReparsePoint, including cloud placeholder tag 0x9000601a. This tag is not a symbolic link/junction or name-surrogate redirection.
- Regression: exact baseline script fails the PowerShell 5.1 Cloud Files-family regression while the repaired script passes; the same regression passes in Windows CI. Evidence: `MDH\_runs\adversarial\mdps-live-001`, `MDH\_runs\ci-34981539523\windows-evidence\ctest-junit.xml`, and `MDH\_evidence\ci-candidate-seal-z.json`.
- Repair rationale: inspect reparse tag without following links; allow only documented non-name-surrogate cloud-placeholder tags; continue refusing symlinks, junctions and unknown reparse tags. No bypass switch or blanket relaxation.
- Files changed: `repo/scripts/archive-local-harness.ps1` now uses uint32 constants and mask `0xFFFF0FFF` to distinguish the documented `0x9000n01A` Cloud Files family from links; `repo/tests/archive-local-harness-reparse-tests.ps1` and the CMake registration provide the regression.
- Focused/complete-suite/new CI/fresh package result: focused baseline-red/repaired-green; Windows CI run `34981539523` PASS with 19 tests (18 executed PASS, 1 documented symlink-privilege skip); exact CI candidate target harness PASS at `MDH\_runs\phase1-z-repaired\target-harness.json`; 121/121 package hashes PASS; tampered script and unsealed file rejected.
- Residual risk: actual OneDrive ancestor reproduction cannot be recreated under the mandated relocated Z workspace; synthetic tag-family regression and direct Z junction/root/state/package rejection cover the safety decision. Agent Manager worker execution was requested with Luna `max` but no runtime session metadata became available, so the independent delegated-review gate remains blocked.

## Investigation Queue (Not Yet Confirmed Product Defects)
- Root canonicalization may erase junction identity before noLinks checks.
- Unknown-ID placeholder includes mutable position.
- Count mismatch validation checks only advertised count greater than actual.
- Running state may survive process death as running rather than interrupted.
- FFprobe metadata-only acceptance may overlook truncated or damaged media.
- GUI startup writes into a sealed application directory.
- Recovery target type/schema conformance and reparse-boundary handling need adversarial tests.
