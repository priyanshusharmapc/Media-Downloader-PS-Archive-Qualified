# Media Downloader PS Archive Mode

This release candidate is built from source commit `4c72054465697b899ede0555e48aff8a7c91c225` by CI run `35116148963`.

The portable application directory is sealed and should not be modified. Archive Root is separate writable operator data. The GUI initializes Archive Root automatically; CLI or scripted use may run `archive-cli.exe preflight <archive-root>` before the first operation.

The portable manifest contains 116 sealed entries. The final report and evidence identify the exact package, source and CI run selected for release.

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
