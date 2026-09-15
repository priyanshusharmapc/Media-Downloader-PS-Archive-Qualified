# Archive Mode And State

## Canonical Identity

The canonical key for a YouTube item is `youtube:<11-character-id>`. Titles, thumbnails and playlist position are metadata. The stable provider identity is not replaced when a title changes, an item becomes private or a playlist no longer returns it.

## Playlist History

Each source has a canonical source record and projections such as `playlist.json`, `items.json`, `catalog.csv`, `missing.csv`, `unavailable.jsonl`, `removed.jsonl`, `video.m3u8` and `audio.m3u8`. Projections are rebuildable views. History records discovery, membership, unavailable status, removal and reappearance. An incomplete scan never implies removal.

## Representations

Video and audio are independent. A canonical item may have one complete representation and one missing or failed representation. A representation becomes canonical only after the downloader succeeds, the file is promoted through the transaction, and FFprobe verification passes. Normalization writes a new canonical representation while retaining the downloaded original when the contract requires it.

## Persistent Layout

```text
ArchiveRoot/
  Video/
  Audio/
  Metadata/
  Playlists/
  State/ArchiveMode/
    items.json
    sources.json
    Imports/{Pending,Accepted,Rejected}/
    Logs/{Activity,Diagnostic}/
    Schemas/
  Temp/
```

`items.json` and `sources.json` are authoritative state. Reports, CSV files, M3U files and playlist JSON are projections. Malformed authoritative state fails closed rather than being silently replaced.

## Transactions And Locks

A root writer lock serializes writers. Downloads and imports stage outside canonical destinations, verify inputs and outputs, then commit state, promotions and receipts together. A journal is recovered on the next start. A stale or interrupted lock is handled according to the recovery rules; concurrent writers do not overwrite each other's bytes.

## Unavailable Sources

An unavailable direct item produces a retryable failed record with extractor diagnostics and no published media. An incomplete playlist scan records partial discovery and retains prior canonical identities. Private, deleted and unavailable items remain historical records where their identity is known.

## Output Safety

Titles and metadata are sanitized for Windows filenames. Traversal, absolute paths, alternate data stream syntax, reserved names, unsafe Unicode separators and linked package paths are rejected. CSV and playlist projections neutralize formula and line injection. Never edit canonical state by hand; use supported import and recovery flows.
