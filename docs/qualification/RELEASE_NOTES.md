# Release Notes

The qualified product commit is `bb949284c15a2ce1128e94d41a931fe407358e64`. GitHub Release `qualification-bb949284c15a2ce1128e94d41a931fe407358e64` holds the portable ZIP (`payload_sha256=8704235825ab2b5f9a3c6fe5eadd052ec08402214f7225d3e040b94f6b0e1cfd`) from run `35613369266`. Commit `77caa99e421ce6d9476d02c3d34c927d963b2528` is a verifier-only follow-up (`GH_REPO` on the ubuntu publisher). Sibling Release `qualification-77caa99e421ce6d9476d02c3d34c927d963b2528` is not the gold product. Gold 4 (endurance and low-memory) is **paused**: `GOLD-4-PAUSED-LOWMEM` (no cmake/ninja/Qt on the L16 host) and `GOLD-4-PAUSED-ENDURANCE` (no operator PlaylistUrl/VideoUrl). This is not a complete gold-state release.

## Candidate

- Product commit `C_main`: `bb949284c15a2ce1128e94d41a931fe407358e64`
- Product Release: `qualification-bb949284c15a2ce1128e94d41a931fe407358e64`
- `R_main`: `35613369266`
- Payload SHA-256: `8704235825ab2b5f9a3c6fe5eadd052ec08402214f7225d3e040b94f6b0e1cfd`
- Verifier-only commit `C_verify`: `77caa99e421ce6d9476d02c3d34c927d963b2528` — not the qualified product
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
