# Archive Root configuration and recovery

This document describes the remediated behavior for MDPS-AUDIT2-001 and
MDPS-AUDIT2-032. The historical audit ledger and qualification reports remain
unchanged and are not evidence for the remediated build.

## Explicit selection

Archive Mode starts unconfigured until an operator selects an existing directory
with **Browse**. The general download folder, application directory, and generic
`Video`, `Audio`, or `State` directories are not evidence of an Archive Root.
Starting the GUI or opening the Archive tab does not create an archive in any of
those locations. Ordinary downloader features are independent of this choice.

A previously configured root remains the selected identity while its removable,
network, or cloud-backed storage is unavailable. Archive actions remain disabled;
the UI displays the configured path and an unavailable-root explanation. Health
refreshes and programmatic import actions do not create a replacement root.
Reconnect the original storage and revisit the Archive tab to resume, or explicitly
use Browse to select a different existing root. The CLI's explicit `init` operation
retains its separate create-new-archive behavior.

Root availability checks use the existing archive link/reparse-point policy. They
do not resolve symlinks or junctions into a different identity to accept them.
Permitted Windows Cloud Files reparse tags retain their existing treatment; this
change does not add a blanket rejection of UNC or cloud-backed paths.

## Selection transaction

A new selection proceeds in this order:

1. Check that the candidate is an absolute, existing directory and passes archive
   filesystem safety checks. Initialize and validate its archive layout/state.
2. Acquire the settings writer lock and atomically commit the new absolute root.
3. Switch the in-memory root and refresh the page.

A validation or settings error leaves the previously accepted root and its
persistent configuration unchanged. Corrupt settings are reported and preserved,
not silently replaced. Concurrent settings writes fail with an explicit retry
message. A failed write is not retried implicitly when the lock or storage later
becomes available.

Initialization may create archive layout in an explicitly selected candidate. If
the subsequent settings commit fails, that candidate data remains available for
inspection or a later explicit retry. It is not deleted as a rollback operation.
No media or canonical records in the previously accepted root are removed.

## Settings compatibility and persistence

The existing user-scoped `archive-mode.ini` remains outside the sealed portable
package. Existing absolute `ArchiveRoot` values retain precedence even while
unavailable. Explicit empty or invalid values do not revive an older root.

The pre-release `ArchiveRootRelativeToApp` setting is still readable relative to
the application directory, including while offline. A successful explicit
selection migrates it to `ArchiveRoot`. Unrelated valid INI settings survive that
commit. Reading a setting does not migrate or rewrite the file.

The writer serializes a private INI snapshot in an owned temporary directory,
with file handles closed before replacement on Windows, then uses `QSaveFile` with direct
write fallback disabled to atomically replace the live settings file. The live
file never has pending `QSettings` changes that a destructor could later retry.
Settings-path links are refused rather than followed to another file. Failures to
create the config directory, obtain its lock, parse the existing settings, stage
bytes, or commit the replacement are surfaced to the caller.

These checks do not make a disconnected mount detectable when the operating
system still presents the same path as an ordinary accessible directory. Users
must reconnect the intended storage, not substitute an unrelated empty directory.
Physical storage failures and arbitrary external writers remain subject to the
archive core's existing integrity checks and recovery mechanisms.

## Regression coverage

`archive-gui-settings-tests` now exercises both the settings contract and actual
Archive widgets in the existing test target. Coverage includes unconfigured launch,
no inferred root, offline legacy/absolute paths, reconnect, rejected corrupt/linked/
missing/relative candidates, successful and repeated selections, restart, preserved
canonical bytes, conflicting settings writers, an unusable settings destination,
corrupt INI preservation, unrelated settings, and package-local immutability.

CTest runs the widget cases with the offscreen Qt platform. The Linux sanitizer
job and Windows qualification job run the same target. Existing tests and their
assertions remain enabled. The qualification settings-write hook now returns a
failure exit code when settings cannot be committed.
