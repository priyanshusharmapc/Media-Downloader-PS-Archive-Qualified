# Release Notes

The current product release is resolved from the latest commit-specific GitHub Release and its `current-release.json`. The historical documented product is `bb949284c15a2ce1128e94d41a931fe407358e64`; the operational baseline before final-candidate normalization is `53e9c8458298ba3396031f2342fd9f4807559311`. Neither is called Gold until every mandatory Gold gate passes.

## Candidate

- Historical product commit: `bb949284c15a2ce1128e94d41a931fe407358e64`
- Current operational baseline: `53e9c8458298ba3396031f2342fd9f4807559311`
- Current candidate identity: resolved from `qualification-<full-source-commit>/current-release.json`
- Gold product: `null` until mandatory Gold gates pass
- Historical `4c720544` / run `35116148963` is not current qualification

## Fixed

- Full FFmpeg stream-integrity validation rejects metadata-readable truncation and corruption.
- Archive Root and parent paths are checked before canonicalization.
- C++ Windows reparse policy is tag-aware and fail-closed.
- GUI settings are outside the sealed application directory.
- Unresolved placeholder identity excludes mutable playlist position.
- Windows reserved-device variants are rejected.
- Inherited upstream publisher workflows were removed.
- Audit2 remediation through `MDPS-AUDIT2-251` is on the product commit.

## Limitations

Provider availability, network behavior and future extractor changes remain external dependencies. Native file-dialog click automation is not used; the packaged executable settings path and package seal are tested through the guarded qualification smoke. Endurance and low-memory are paused as named above; they are not PASS.
