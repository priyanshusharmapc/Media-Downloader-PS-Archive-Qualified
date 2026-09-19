# Archive Mode And State

YouTube identities use `youtube:<id>` when a provider ID exists. Playlist occurrences are historical memberships and may point to one canonical item. Incomplete discovery never implies removal.

Video and audio are independent representations. A representation becomes `complete` only after successful download, transactional promotion, profile validation and full FFmpeg stream-integrity validation. Reports, CSV files, M3U files and playlist JSON are projections; `items.json` and `sources.json` are authoritative.

Unresolved placeholders use stable source/title/URL material rather than mutable playlist position. Duplicate unresolved occurrences receive separate occurrence keys and prior matches preserve their identity during reorder/insertion.

Archive Root layout:

```text
ArchiveRoot/
  Video/ Audio/ Metadata/ Playlists/
  State/ArchiveMode/
    items.json sources.json Imports/{Pending,Accepted,Rejected}
    Logs/{Activity,Diagnostic} Temp/
```

State is transactional and malformed state fails closed. A root writer lock serializes writers. Journal recovery, preimage checks and receipt hashes protect interrupted operations.
