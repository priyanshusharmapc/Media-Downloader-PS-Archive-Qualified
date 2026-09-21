# Media Downloader PS Archive Mode

The qualified product commit is `bb949284c15a2ce1128e94d41a931fe407358e64`. GitHub Release `qualification-bb949284c15a2ce1128e94d41a931fe407358e64` holds the portable ZIP (`payload_sha256=8704235825ab2b5f9a3c6fe5eadd052ec08402214f7225d3e040b94f6b0e1cfd`) from run `35613369266`. Commit `77caa99e421ce6d9476d02c3d34c927d963b2528` is a verifier-only follow-up (`GH_REPO` on the ubuntu publisher). Sibling Release `qualification-77caa99e421ce6d9476d02c3d34c927d963b2528` is not the gold product. Gold 4 (endurance and low-memory) is **paused**: `GOLD-4-PAUSED-LOWMEM` (no cmake/ninja/Qt on the L16 host) and `GOLD-4-PAUSED-ENDURANCE` (no operator PlaylistUrl/VideoUrl). This is not a complete gold-state release.

A later documentation commit on `main` is not the product. Do not retag `qualification-bb949284c15a2ce1128e94d41a931fe407358e64`.

The portable application directory is sealed and should not be modified. Archive Root is separate writable operator data. The GUI initializes Archive Root automatically; CLI or scripted use may run `archive-cli.exe preflight <archive-root>` before the first operation.

The portable manifest is sealed. The GitHub Release named above identifies the exact package, source and CI run selected for this product.

Security includes traversal, symlink, junction and reparse-point rejection. The project license and third-party notices remain part of the source/package distribution.

## Contents

- `INSTALLATION_AND_USER_GUIDE.md`
- `ARCHIVE_MODE_AND_STATE.md`
- `RECOVERY_BACKUP_SECURITY.md`
- `TESTING_QUALIFICATION.md`
- `SOURCE_MAP.md`
- `RELEASE_NOTES.md`
- `AUDIT_REMEDIATION.md`

## References

- https://github.com/priyanshusharmapc/Media-Downloader-PS
- https://github.com/yt-dlp/yt-dlp
- https://ffmpeg.org/legal.html
- https://www.qt.io/licensing/
