# Media Downloader PS Archive Mode

This documentation describes the Windows portable candidate built from source commit `261fcef529be7982b9048a39161941c1572b3044`. It is the Archive Mode qualification candidate for GitHub Actions run `35000478514`.

## Contents

- `INSTALLATION_AND_USER_GUIDE.md`: installation, GUI and CLI operation.
- `ARCHIVE_MODE_AND_STATE.md`: persistent state, playlist history and media rules.
- `RECOVERY_BACKUP_SECURITY.md`: Recovery Packages, backup, relocation and filesystem safety.
- `TESTING_QUALIFICATION.md`: build, test, evidence and release controls.
- `SOURCE_MAP.md`: repository map and authority boundaries.
- `RELEASE_NOTES.md`: candidate identity, hashes, limitations and decision.

## Quick Start

1. Extract the portable folder to a user-writable location.
2. Keep the application folder and Archive Root separate. The Archive Root contains persistent operator data.
3. Start `media-downloader.exe` for the GUI, or use `archive-cli.exe` for scripted Archive Mode operations.
4. For CLI or scripted use, run `archive-cli.exe preflight <archive-root>` before the first operation. The GUI initializes the Archive Root automatically when the Archive tab opens.

The portable package is commit- and hash-bound. Do not add, remove or replace files inside it after extraction. Keep backups of the Archive Root, not only the application folder.

## References

- [Repository](https://github.com/priyanshusharmapc/Media-Downloader-PS)
- [Qt licensing](https://www.qt.io/licensing/)
- [yt-dlp](https://github.com/yt-dlp/yt-dlp)
- [FFmpeg](https://ffmpeg.org/legal.html)

## Support Boundary

The package supports Windows x64 and depends on the bundled yt-dlp, Deno, FFmpeg and FFprobe tools. YouTube availability, extractor behavior, network access and provider policy remain external dependencies. A failed or unavailable source is retained as history; it is not treated as playlist deletion.
