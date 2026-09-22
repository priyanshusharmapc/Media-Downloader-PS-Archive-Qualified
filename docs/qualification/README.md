# Media Downloader PS Archive Mode

The current qualification release is resolved from the latest commit-specific GitHub Release named `qualification-<full-source-commit>`, whose `current-release.json` binds the exact source commit, workflow run, package and SHA-256. The historical documented product is `bb949284c15a2ce1128e94d41a931fe407358e64`, released as `qualification-bb949284c15a2ce1128e94d41a931fe407358e64`. The latest operational baseline at this normalization is `53e9c8458298ba3396031f2342fd9f4807559311`, released as `qualification-53e9c8458298ba3396031f2342fd9f4807559311`. Neither identity is a Gold product until all mandatory Gold gates pass.

Historical releases must not be retagged or clobbered. A later documentation, verifier or qualification commit is a new candidate when it changes the source tree.

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
