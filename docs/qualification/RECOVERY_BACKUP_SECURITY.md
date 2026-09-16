# Recovery, Backup And Security

Recovery Packages are direct unlinked directories with schema-versioned `manifest.json`, safe package-relative files and provenance. Validation rejects traversal, absolute paths, alternate data streams, reserved names, linked paths and unsafe reparse boundaries. Accepted packages are normalized, FFprobe/profile and full-stream verified, receipted and moved to `Imports/Accepted`; invalid packages are rejected.

Stop all writers before backup. Copy the complete Archive Root, restore to a fresh location, run `preflight` and `verify-item`, then resume. Archive Root paths are relative and relocation is supported.

The C++ safety layer validates the requested root before canonical resolution, inspects Windows reparse tags, permits the documented Cloud Files `0x9000n01A` family where policy allows, and rejects symbolic links, junctions, name-surrogate and unknown unsafe reparse points. The application package is immutable; GUI settings are stored in the per-user Qt application-configuration location.

The product does not guarantee provider availability, malware scanning, DRM access or future extractor compatibility. Never edit canonical JSON by hand.
