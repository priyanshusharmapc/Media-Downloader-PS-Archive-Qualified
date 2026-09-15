# Installation And User Guide

## Installation

Extract the complete `Media-Downloader-PS` directory. Preserve its relative layout and use a location where the operator can execute binaries. The application directory is not the Archive Root and should not be used as the only backup location.

The portable folder contains the GUI, Archive CLI, Qt runtime, FFmpeg/FFprobe, yt-dlp, Deno, Archive Mode schemas and the package manifest. `SHA256SUMS.txt` is the integrity boundary.

## GUI Workflow

1. Start `media-downloader.exe`.
2. Select or create an Archive Root outside the application directory.
3. Add a playlist or video source and start discovery.
4. Review discovered membership and availability before synchronization.
5. Download video and audio independently, then verify the item.
6. Use the playlist, missing and unavailable views to distinguish current membership from historical identity.

Archive Root selection is persistent operator data. Moving the application does not move the Archive Root. Use the backup and relocation procedure in `RECOVERY_BACKUP_SECURITY.md` when changing storage locations.

## CLI

Run commands from the portable directory:

```text
archive-cli.exe preflight <archive-root>
archive-cli.exe validate <archive-root> <package-directory>
archive-cli.exe ingest-pending <archive-root>
archive-cli.exe scan <archive-root> <youtube-playlist-url> [display-name]
archive-cli.exe sync-item <archive-root> <youtube-video-url>
archive-cli.exe verify-item <archive-root> <youtube-video-url>
```

`preflight` verifies the bundled runtime tools and initializes a safe root. `scan` records source discovery and reconciles membership. `sync-item` downloads representations transactionally. `verify-item` rechecks state, files, hashes and FFprobe metadata. `validate` and `ingest-pending` operate only on direct, unlinked Recovery Package directories under `State/ArchiveMode/Imports/Pending`.

## Results

Exit code zero means the requested operation completed. A nonzero exit means the operation failed or the source was incomplete. Inspect the diagnostic and activity logs before retrying. A failed operation must not be converted to PASS by inspecting only a projection file.

## Common Operations

### Re-download a missing representation

Run `sync-item` for the canonical video URL, then `verify-item`. Existing valid representations are preserved and the missing representation is retried.

### Inspect unavailable history

Run `scan` again when the provider becomes available. Existing playlist occurrences remain historical when discovery is incomplete or a source is unavailable.

### Move the Archive Root

Stop the GUI and all CLI processes, copy the complete Archive Root, verify it at the new location, then configure the GUI to use the new root. Never merge two writers into the same root without the root writer lock.
