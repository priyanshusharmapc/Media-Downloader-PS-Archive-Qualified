# Testing And Qualification

The qualified product commit is `bb949284c15a2ce1128e94d41a931fe407358e64`. GitHub Release `qualification-bb949284c15a2ce1128e94d41a931fe407358e64` holds the portable ZIP (`payload_sha256=8704235825ab2b5f9a3c6fe5eadd052ec08402214f7225d3e040b94f6b0e1cfd`) from run `35613369266`. Commit `77caa99e421ce6d9476d02c3d34c927d963b2528` is a verifier-only follow-up (`GH_REPO` on the ubuntu publisher). Sibling Release `qualification-77caa99e421ce6d9476d02c3d34c927d963b2528` is not the gold product. Gold 4 (endurance and low-memory) is **paused**: `GOLD-4-PAUSED-LOWMEM` (no cmake/ninja/Qt on the L16 host) and `GOLD-4-PAUSED-ENDURANCE` (no operator PlaylistUrl/VideoUrl). This is not a complete gold-state release.

Product qualification for `C_main` is the Archive Qt6 **push** run `35613369266` (`R_main`) on `bb949284c15a2ce1128e94d41a931fe407358e64`: Linux and Windows jobs executed real steps, including sanitizer CTest and portable GUI launch smoke. The ubuntu `publish-qualification-evidence` job on that run failed for missing `GH_REPO`; the product Release was already created by the Windows job. Commit `77caa99e421ce6d9476d02c3d34c927d963b2528` only set `GH_REPO` on the verifier. Do not treat `C_verify` or sibling `qualification-77caa99e421ce6d9476d02c3d34c927d963b2528` as the product.

The media suite uses real FFmpeg-generated fast-start MP4/M4A fixtures. Metadata-readable truncation is rejected, corrupted canonical media is repaired by sync, valid media passes, and full stream consumption is required.

CI YouTube discovery remains advisory (`exit 0`) and is not endurance evidence. Historical `4c720544` / run `35116148963` numbers are not current qualification.

`GOLD-4-PAUSED-LOWMEM` and `GOLD-4-PAUSED-ENDURANCE` are operator gaps, not PASS. A 60-minute identity-bound endurance run and serial low-memory CTest were not executed for this product. Any unavailable provider or host-specific operation is recorded as blocked, not inferred as PASS.
