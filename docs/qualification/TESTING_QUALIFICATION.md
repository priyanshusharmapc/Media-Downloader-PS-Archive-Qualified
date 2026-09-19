# Testing And Qualification

The final candidate source is `4c72054465697b899ede0555e48aff8a7c91c225`, qualified by CI run `35116148963`.

Windows CI executed 29/29 tests with no skips, including real root/parent/link/reparse tests, media-integrity tests, GUI settings/package-seal tests and the 23-case integration suite. Linux normal and sanitized suites passed; ASan and UBSan completed without diagnostics. The sealed portable manifest contains 116 entries.

The media suite uses real FFmpeg-generated fast-start MP4/M4A fixtures. Metadata-readable truncation is rejected, corrupted canonical media is repaired by sync, valid media passes, and full stream consumption is required.

Live qualification used three real YouTube items: `0fvbzgnVO2Y`, `g3Mh8Hws-jo` and `Ynv_WYO_slw`. Video/audio representations were harvested and verified with the code-equivalent final candidate, then final-package target verification was run against the exact release artifact. Only documentation changed between the harvest and final source. Unavailable-source behavior, Recovery Packages, tamper rejection, transactions, crash recovery, locks, concurrency, relocation, scale and projections were exercised.

The identity-bound endurance run lasted at least 60 minutes and recorded iterations, complete media-set hashes, package identity, temporary residue, journal residue and failures. It executed before documentation-only fix-list commits; the binding proves no executable or test-code change afterward. The final report lists exact evidence paths and hashes. Any unavailable provider or host-specific operation is recorded as blocked, not inferred as PASS.
