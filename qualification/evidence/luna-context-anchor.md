# Luna Context Anchor

Last refresh: 2026-09-15T23:25:00+05:30.

## Mission

Qualify, harden, repair, document and package Media Downloader PS Archive Mode. Release only if every applicable mandatory software, safety, GUI, package and documentation gate passes. Never substitute unexecuted or model-unverified work with PASS.

## Filesystem and Authority

- HARNESS_PARENT: `Z:\KILOCODE WORK\Media Downloader Harness`
- WORK_ROOT: `Z:\KILOCODE WORK\Media Downloader Harness\MDH`
- OUTPUT_ROOT: `Z:\KILOCODE WORK\Media Downloader Harness\MDH\Output` (promotion-only)
- BUNDLE_ROOT: `Z:\KILOCODE WORK\Media Downloader Harness\Media-Downloader-PS-Harness-Drive-acfd2a86` (immutable)
- QUALIFIED_PORTABLE: `BUNDLE_ROOT\READY-TO-RUN-WINDOWS\Media-Downloader-PS` (immutable)
- BUNDLED_SOURCE: `BUNDLE_ROOT\repository-source` (source snapshot, not Git)
- DOC_PROJECT_ROOT: `HARNESS_PARENT\DOC\40 - Software, AI & Infrastructure Projects-20260915T070932Z-1-001\40 - Software, AI & Infrastructure Projects\Media Downloader PS` (immutable)
- Repository: `https://github.com/priyanshusharmapc/Media-Downloader-PS.git`
- Branch: `luna-max-mdps-live-001`
- Known qualified baseline: `acfd2a86d5af6ae07c4352461071e73a08b2b000`
- Supplied CI run: `34886206216`; repaired CI runs: `34981539523`, `35000478514`
- Source authority order: exact tested executable source, schemas/contracts, runtime contract, verified behavior, maintained repository docs, active CI, external DOCX, historical reports.

## Model and Orchestration

- Primary: GPT-5.6 Luna, `max`, as identified for this assignment by the current system/user context.
- Agent Manager catalog: GPT-5.6 Luna is available from `openai` with variants `none`, `low`, `medium`, `high`, `xhigh`, `max`.
- Delegated workers: final Agent Manager session `ses_f59c3626fffeXmUD68zZVPRta1` was dispatched explicitly as `GPT-5.6 Luna (openai) / max` and returned PASS with no blocker.
- No alternate model, provider or fallback is permitted. If pinned routing cannot be verified, the affected gate is `BLOCKED - Luna Max routing not enforceable`.
- Host scripts remain Windows PowerShell 5.1 compatible unless PowerShell 7 is explicitly verified.

## Product and Invariants

Media Downloader PS is a Qt/C++ upstream fork with native Archive Mode. Archive Root is persistent operator data outside the application folder. Canonical item state and playlist history are authoritative; reports and M3U/CSV files are projections. YouTube IDs are stable canonical identities, playlist occurrences are historical memberships, and incomplete discovery never implies removal. Video and audio representations are independent and must be normalized and FFprobe-verified before canonical promotion. Malformed state fails closed; transactions, journals and root writer locks serialize and recover safely; Recovery Packages are additive and immutable after Accepted; paths reject traversal, ADS, reserved names and unsafe reparse escapes; portable acceptance is commit/hash-bound; exact requested items and idempotent canonical hashes are required.

## Current Phase and Evidence

- Current phase: release-ready candidate and immutable promotion complete.
- Verified on Z: supplied integrity, source comparison, DOCX re-verification, Cloud Files baseline-red/repaired-green regression, exact final candidate target harness, exact final-candidate three live items, exact final-candidate unavailable source, Recovery Packages, tamper rejection, backup/restore, relocation, filesystem safety, scale, soak, GUI smoke and documentation/public-source QA. CI run `35000478514` passed Windows/Linux builds, tests, sanitizers and package gates; Windows CTest has 19 tests, 0 failures and 0 skips, including the real symlink boundary test.
- Known defect: `MDPS-LIVE-001` is resolved at commit `261fcef529be7982b9048a39161941c1572b3044`. Final release-ready status is PASS for `Output-Release-Ready`.
- Evidence roots: `MDH\_evidence`, `MDH\_runs`, `MDH\_logs`, `MDH\_staging`.

## Required Gates

Baseline integrity, repository/source/DOCX comprehension, target-host acceptance, three-item live operation, unavailable-source behavior, adversarial discovery/media/report/log tests, crash/transaction recovery, locking/concurrency, filesystem/reparse safety, Recovery Packages, package tampering, GUI, backup/restore/relocation, scale, soak, regression/integration/sanitizers/Windows qualification, documentation QA/fresh-reader review, public-source cleanliness and exact-byte Output acceptance.

## Output Contract

Final Output contains a clean tested Windows portable, curated Documentation, publication-ready GitHub-Source, runtime/source ZIPs, release notes, release qualification report and outer SHA-256 manifest. The prior Output and `Output-Release-Ready` are immutable after final acceptance. A mandatory failure or blocked gate means `NOT RELEASE READY`.

## Documentation Gate

Documentation must describe the final tested implementation, including installation, user and GUI operation, CLI, state/history, media pipeline, transactions/locking/recovery, Recovery Packages, backup/restore, troubleshooting, limitations, security, build/test/qualification/release, source map, valid examples, links, licensing and no local/private-data leakage.
