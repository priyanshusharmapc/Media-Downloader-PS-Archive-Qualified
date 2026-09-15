# Recovery, Backup And Security

## Recovery Packages

A Recovery Package is a direct unlinked directory containing `manifest.json`, optional media under a package-relative `files/` directory, metadata and optional evidence. The schema is shipped in `archive-resources/recovery-package.schema.json`.

Validation requires schema version 1, a safe package identity, a known canonical target, valid provenance and safe nonempty regular files. A package cannot overwrite an already complete representation. Ingestion normalizes and FFprobe-verifies media, writes a receipt with manifest and file SHA-256 values, and moves accepted evidence to `Imports/Accepted`. Invalid packages move to `Imports/Rejected`; retryable failures remain pending.

Never use a linked package directory, a path outside `Pending`, a package-relative traversal path or a package that has been modified after validation.

## Backup And Restore

1. Stop all writers.
2. Copy or archive the complete Archive Root, including `State`, `Metadata`, `Playlists`, `Video`, `Audio` and `Temp`.
3. Restore to a fresh location.
4. Run `preflight` and `verify-item` for representative canonical items.
5. Compare authoritative state and receipt hashes before resuming operations.

The qualification evidence restored a complete root from a ZIP, verified the item, and verified the same item after relocation. Archive Root paths are relative; host-specific absolute paths are not stored in canonical state.

## Reparse Points And Links

The local harness inspects every path component. Symbolic links, junctions, name-surrogate reparse tags and unknown reparse tags are refused. Microsoft Cloud Files placeholders in the documented `0x9000n01A` family are allowed without following them. The classifier uses uint32 constants and mask `0xFFFF0FFF`; the OneDrive-style `0x9000601A` tag is covered by a Windows PowerShell 5.1 regression.

## Threat Model

The product treats Archive Root, package directories, downloader output, metadata and projections as hostile inputs. It protects against traversal, linked paths, malformed JSON, corrupted journals, concurrent writers, partial downloads, stale locks, archive tampering, CSV/playlist injection and incomplete media. It does not provide DRM circumvention, provider account access, malware scanning or a guarantee that external URLs remain available.

## Tamper Response

Do not repair a hash mismatch in place. Extract a fresh package and rerun the manifest verification. The target-host harness rejects modified sealed files and unlisted files before running the downloader or writing Archive Root data.
