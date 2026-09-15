# Decisions

## D001: Real capabilities over requested labels
The primary model is identified by installed instructions as openai/gpt-6-astra. No mode change is used. The runtime reasoning selector cannot be inspected with the available tools; Xhigh is not independently attested. Native Task exists but has no per-task model selector. Initial combined-name searches incorrectly conflated model names with reasoning variants. Corrected queries on 2026-09-15T12:07Z found GPT-5.6 Luna (`max`) and GPT-5.6 Terra (`medium`) offered by openai and other listed providers. This establishes catalog availability, NOT worker execution. Task still exposes only general/explore with no model/variant override. No Kilo CLI on PATH. Do not silently substitute or claim a model ran. Primary executes pending a supported enforceable routing path within the MDH write boundary; independent requested-model reviews remain blocked until actual routing is verified.

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
