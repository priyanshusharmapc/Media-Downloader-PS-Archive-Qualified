# Testing And Qualification

## Build

The release candidate was built by GitHub Actions run `35000478514` from commit `261fcef529be7982b9048a39161941c1572b3044`. Windows CTest registered 19 tests and all 19 executed PASS, including the real symbolic-link package-boundary test. The Windows integration suite ran 22 cases covering discovery, media normalization, unavailable sources, transactions, crash recovery, locks, concurrency, corruption, injection, Recovery Packages, tamper rejection and idempotency.

The Linux build and sanitized test job passed its registered CTest suite and integration tests. AddressSanitizer and UndefinedBehaviorSanitizer completed without a reported finding in the CI evidence.

## Live Qualification

The live fallback playlist was probed at runtime. Three currently accessible items were synchronized into a disposable Archive Root. Video and audio were independently FFprobe-verified. Fresh-process reruns preserved canonical identity, media hashes and projections without duplicate canonical keys.

The exact sealed CI candidate passed target-host harness acceptance on the relocated Z workspace. yt-dlp, FFmpeg, FFprobe and Deno were found from the candidate. The repaired harness performed preflight, scan, sync, verify and an idempotent rerun. The same exact candidate completed the three-item live cycle with video/audio FFprobe success and stable hashes.

## Filesystem And Recovery

Fresh Z-root tests covered junction root rejection, linked state rejection, linked Recovery Package rejection, long paths, unavailable sources, malformed state, backup/restore, relocation, Recovery Packages, tampering, 1,000-item scale and a 30-cycle soak. The authoritative Windows runner enabled symbolic-link qualification and the linked-package test executed and passed.

## Evidence Rules

Evidence records commands, exit codes, timestamps, hashes and exact paths. Historical C-path evidence is not used as relocated acceptance evidence. The final candidate was re-hashed after GUI and target-host tests; all 116 manifest entries matched and no unsealed file remained.

## Release Gate

Software, package, target-host, live, recovery, safety, CI and documentation checks are individually recorded in the qualification report. The final report states the release decision and remaining blockers explicitly.
