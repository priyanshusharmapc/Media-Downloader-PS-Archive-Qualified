# Installation And User Guide

Extract the complete `Media-Downloader-PS` directory to a user-writable location. Start `media-downloader.exe` for GUI operation. Use `archive-cli.exe` for automation.

Create an Archive Root outside the application directory. It stores persistent media, metadata, playlist history, state, logs, imports and temporary transaction data. The GUI initializes the selected root automatically. CLI users may run:

```text
archive-cli.exe preflight <archive-root>
archive-cli.exe scan <archive-root> <youtube-playlist-url> [display-name]
archive-cli.exe sync-item <archive-root> <youtube-video-url>
archive-cli.exe verify-item <archive-root> <youtube-video-url>
archive-cli.exe validate <archive-root> <package-directory>
archive-cli.exe ingest-pending <archive-root>
```

`verify-item` checks state, representation paths, profile metadata and full FFmpeg stream consumption. It does not compare a durable content hash in state. The qualification harness uses SHA-256 for package sealing and idempotency evidence.

Keep the portable directory immutable. Back up and relocate the Archive Root separately. Provider/network/extractor availability is external and can change.
