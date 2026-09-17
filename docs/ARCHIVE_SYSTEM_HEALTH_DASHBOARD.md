# Archive System Health Dashboard

The Archive tab is designed as an operational control surface first and a media browser second. The first screen should answer a simple question: **is Archive Mode functioning correctly right now?**

The system-health dashboard therefore appears above playlist and individual-item details. It summarizes the parts of the archive that can affect correctness, safety, or operator confidence without requiring the operator to inspect a specific video.

## What the dashboard shows

### Overall

The top-level state is one of:

- `HEALTHY`: no system-level problem was detected by the dashboard checks.
- `ATTENTION`: Archive Mode is usable, but one or more conditions deserve operator review.
- `ERROR`: a condition required for safe operation is missing or inconsistent, such as an inaccessible/non-writable Archive Root, canonical-state integrity failure, or a missing required runtime.
- `NOT CONFIGURED`: no Archive Root is currently selected.

The dashboard is an operational summary, not a replacement for qualification, backups, explicit media verification, or the Windows local harness.

### Archive Root

The dashboard checks that the configured Archive Root exists, is a directory, and is writable. A missing or non-writable root is an `ERROR` condition.

### Canonical state

The dashboard reads the managed source registry, canonical item registry, and per-source playlist state and reports the number of sources, canonical items, and occurrence references.

It also surfaces integrity conditions that should not be silently hidden:

- duplicate canonical keys;
- playlist references that do not resolve to canonical items;
- missing or duplicate occurrence `entry_key` values;
- a previously scanned managed source whose `items.json` file is missing;
- registry or playlist-state parse/validation failures;
- representation state that remains `running` while the GUI is idle;
- failed or interrupted representations;
- sources whose latest scan reports an error or incomplete result.

These checks are intentionally about **software/state correctness**, not about whether a particular song is interesting or available.

### Runtime tools

Archive Mode reports how many of the four required runtime tools resolve:

- yt-dlp;
- FFmpeg;
- FFprobe;
- Deno.

The dashboard also reports how many of the resolved tools are inside the application package. A missing tool is an `ERROR`. A tool that resolves outside the package is surfaced as `ATTENTION` because the runtime is then dependent on the host environment rather than only the packaged toolset.

### Disk headroom

The dashboard displays available storage and the percentage of the backing volume that remains free. It raises an attention condition when available space is below 5 GiB or below 5% of the volume.

This threshold is an operator warning, not a guarantee that a particular download will fit. Large archives still require capacity planning for video, audio, `Temp/`, recovery normalization, and transaction work.

### Writer lock

Archive mutation is single-writer.

The dashboard distinguishes:

- the lock being held by the current GUI operation;
- the lock being available;
- the lock being held by another writer.

A lock owned by another process is shown as `ATTENTION`. Do not delete the lock file merely to clear the dashboard status. Determine which GUI or CLI process owns the archive first.

### Recovery imports

The dashboard reports the number of package directories currently under:

- `Pending`;
- `Accepted`;
- `Rejected`.

These counts are shown as operational workload. A non-zero historical Accepted or Rejected count is not by itself proof that the software is unhealthy.

### Latest scan

The dashboard shows the latest recorded scan time across registered sources and the number of sources whose latest state reports an error or incomplete scan.

A failed or incomplete discovery result is surfaced because destructive reconciliation must never be inferred from an incomplete source observation.

## Attention vs archive workload

The dashboard deliberately separates **system alerts** from **archive work**.

System alerts are conditions that may indicate incorrect software operation, unsafe state, environment drift, or a failed/incomplete workflow.

Archive workload is expected content-level work, including:

- canonical items whose video or audio representation is incomplete;
- items awaiting external recovery;
- unavailable source items;
- Pending recovery packages;
- Rejected recovery packages that may need review.

A large workload does not automatically mean Archive Mode is malfunctioning.

## Current Operation panel

The right-hand panel shows what Archive Mode is doing now instead of only displaying a generic busy spinner.

For playlist scan/sync operations the GUI reports stages that correspond to actual implementation boundaries:

1. `Discovery`
2. `Reconcile`
3. `Media sync + verify + commit`
4. `Projection`
5. `Finalizing`

For media work, progress is reported against the number of eligible items in the current source. For multi-source work, the detail text also identifies the source index. Failure count remains visible while the operation continues.

`Media sync + verify + commit` is intentionally represented as one stage because the current `MediaExecutor::syncItem()` call owns that sequence internally. The UI must not pretend it can observe a finer-grained boundary that the implementation does not expose.

Recovery-package processing reports a `Recovery imports` stage while Pending packages are validated and ingested.

When an operation finishes the panel retains the terminal state (`COMPLETED`, `COMPLETED WITH FAILURES`, `STOPPED`, or `ERROR`) and a compact result summary so the user can see what happened without opening an individual item.

## Stop behavior

`Stop After Current` remains cooperative. The dashboard changes its operation detail to show that a stop has been requested, but the current item is allowed to finish safely before the remaining queue is abandoned.

The dashboard does not describe a stop as immediate cancellation because that would misrepresent the implementation.

## Playlist and item details

Playlist navigation, search/filter controls, the item table, Source/Archive/History/Recovery tabs, and Activity view remain available below the system dashboard.

They are intentionally secondary in the visual hierarchy. The operator should be able to determine system health, current operation, failures, runtime availability, lock state, storage headroom, and integrity warnings before drilling into song-level metadata.

## Operator response to ERROR or ATTENTION

When the dashboard shows `ERROR`, do not assume that retrying downloads will repair the problem. Read the specific health row and the Attention text first.

Typical responses include:

- restore or reselect the correct Archive Root;
- resolve a non-writable filesystem or low-space condition;
- restore the qualified packaged runtimes;
- stop an unexpected concurrent archive writer;
- preserve state before investigating canonical or playlist integrity errors;
- review Activity/Diagnostic logs after failed or incomplete source scans;
- process or inspect Pending/Rejected recovery packages through the documented recovery workflow.

For incident handling, backup, transaction recovery, qualification, and CLI preflight procedures, continue with [ARCHIVE_OPERATIONS.md](ARCHIVE_OPERATIONS.md) and [ARCHIVE_TESTING.md](ARCHIVE_TESTING.md).
