# Release Notes

## Candidate

- Source commit: `4c72054465697b899ede0555e48aff8a7c91c225`
- CI run: `35116148963`
- Windows tests: 29/29 PASS, zero skips
- Linux and sanitizer tests: PASS
- Portable package internal manifest: 116/116 entries verified

## Fixed

- Full FFmpeg stream-integrity validation rejects metadata-readable truncation and corruption.
- Archive Root and parent paths are checked before canonicalization.
- C++ Windows reparse policy is tag-aware and fail-closed.
- GUI settings are outside the sealed application directory.
- Unresolved placeholder identity excludes mutable playlist position.
- Windows reserved-device variants are rejected.
- Inherited upstream publisher workflows were removed.
- Endurance and package evidence are identity-bound.

## Limitations

Provider availability, network behavior and future extractor changes remain external dependencies. Native file-dialog click automation is not used; the packaged executable settings path and package seal are tested through the guarded qualification smoke.
