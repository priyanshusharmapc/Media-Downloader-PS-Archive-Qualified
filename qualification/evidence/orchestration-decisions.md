# Decisions

## D001: Real capabilities over requested labels
The current assignment primary is GPT-5.6 Luna at `max`. Agent Manager catalog queries on 2026-09-15 found GPT-5.6 Luna (`max`) through `openai`. This establishes catalog availability, not worker execution. Native Task does not expose a per-task model/variant selector, so it was not used for delegated model-sensitive work. Agent Manager dispatches were explicitly pinned to Luna/openai/max, but no runtime session metadata became visible. Do not silently substitute or claim a delegated model ran; the affected independent-review gate remains blocked.

## D002: Preserve all originals
All generated scripts, extraction, Git, logs, media and packages go under MDH. Original hashes are read-only. Scripts use explicit Windows PowerShell 5.1. Output is not a build or test directory.

## D003: Honest qualification
Historical CI is evidence only for its bound baseline. Failed or unavailable mandatory gates prevent release. No manual canonical-state or qualification-identity repair. Production changes require failing regression, minimal fix, complete requalification and fresh artifact acceptance.

## D005: Source artifact conversion and legacy reference scope
The 239-file Git archive and supplied snapshot differ in 186 files solely by CRLF/LF conversion. No missing, extra or substantive source differences were found. Report byte-level DIFFERENCES FOUND, substantive MATCH. DOCX guidance about old branches, Drive-only publication and source layout is historical and superseded by the current master prompt and exact implementation. No original DOCX is changed.

## D006: Standalone preflight versus empty-root harness
Preflight initializes canonical root state. Preserve the complete preflight root by moving it intact to PreflightRoot, then run the first harness on a new empty ArchiveRoot. Do not delete canonical files or pass AllowExistingArchive to work around the harness's empty-root requirement.

## D004: Checksum filename anomaly
COMPLETE-ZIP.sha256 contains a hash for a historical path ending in Harness-Core rather than the actual Complete ZIP name. Compare the supplied digest directly to the complete ZIP and record both digest and naming discrepancy. A matching digest establishes content integrity, not publisher authenticity.

## D007: Cloud Files reparse classification
The staged `MDPS-LIVE-001` repair initially used `0xFFFFF0FF`, which both ignored the wrong nibble and compared a uint32 result with a signed PowerShell literal. The OneDrive tag `0x9000601A` was therefore still rejected. The corrected classifier uses uint32 constants and mask `0xFFFF0FFF`, accepts only the documented `0x9000n01A` Cloud Files family, and continues to reject symlink, junction and unknown tags. A Windows PowerShell 5.1 regression fails against the exact baseline script and passes against the repaired script.

## D008: Relocated full-GUI build limitation
The authoritative CI configure command was mirrored without the obsolete `--preprocessor=gcc` override. Archive CLI/core/hardening/fake-tool targets build and test on Z:. The full Windows GUI target still fails in MinGW `windres` because the relocated workspace path contains spaces and windres's implicit preprocessor invocation misquotes it. This is a local environmental/toolchain limitation, not a PASS; fresh Windows CI is required for the GUI/package gate.

## D009: Local CTest scope
The fresh Z CTest run executed 19 registered tests: 18 PASS, one host-dependent `archive-linked-package-file` test skipped with its documented Windows privilege return code 77. The Python integration test passed and includes concurrency, killed-writer, transaction, corruption, injection, recovery, and junction scenarios. The skipped symlink-specific case remains a mandatory filesystem evidence item for independent Windows/packaged qualification.

## D010: Final release decision
The repaired CI candidate, exact relocated target harness, live three-item run, Recovery Package flow, tamper tests, backup/restore, relocation, scale, soak, GUI smoke, documentation QA and immutable Output acceptance all passed. Final status remains `NOT RELEASE READY` because the required Agent Manager Luna Max worker execution cannot be verified and the exact Windows symlink privilege test was skipped. The decision is evidence-backed and must not be promoted to release-ready without closing both gates.

## D011: Exact-candidate live follow-up
After Output acceptance, the exact sealed CI candidate itself was run against the three selected live items and the synthetic unavailable item. All three video/audio sync, verify, FFprobe, restart and hash-idempotency checks passed; unavailable direct sync returned 1, incomplete playlist scan returned 3, failed identity/error state was retained and no media was published. Evidence remains outside immutable Output at `MDH\_runs\live\three-item-repaired-z\acceptance.json` and `MDH\_runs\live\unavailable-repaired-z\acceptance.json`.

## D012: Final release-ready decision
Commit `261fcef529be7982b9048a39161941c1572b3044` and CI run `35000478514` passed 19/19 Windows tests with the substantive symbolic-link boundary test, Linux tests and sanitizers, exact candidate target/live/unavailable runs, package seals, documentation QA and independent GPT-5.6 Luna max review. `Output-Release-Ready` passed final read-only acceptance with 392 manifest entries. Final status is `RELEASE READY` for the new promotion; the earlier `Output` remains immutable historical NOT RELEASE READY evidence.
