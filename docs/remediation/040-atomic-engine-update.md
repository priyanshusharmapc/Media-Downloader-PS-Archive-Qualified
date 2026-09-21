# MDPS-AUDIT2-040 remediation

## Defect

Engine updates removed the currently working executable or folder before the replacement had been successfully installed. A rename, extractor, permission, storage, antivirus or layout failure could therefore convert a recoverable update failure into an engine outage.

## Correction

Standalone engine payloads now use rollback-capable promotion. At the final commit boundary, the prior destination is renamed to a unique sibling backup, the already-downloaded replacement is promoted, and any promotion failure restores the prior destination before the updater reports failure. The old backup is deleted only after successful promotion.

Archive-form updates no longer extract into the live engine directory. A unique `.mdps-update-stage-*` directory is created under the engine root, extraction and archive-folder normalization complete entirely there, and only then are staged top-level entries promoted. Existing live entries are retained in a rollback directory until every staged entry has moved successfully. A failed extraction never touches the live engine tree. A failed promotion removes incomplete new entries and restores all saved live entries. If restoration itself fails, the rollback backup is deliberately retained on disk and its path is included in the reported error so recovery evidence is never deleted.

The prior MDPS-AUDIT2-041 executable-permission fix is preserved: permissions are applied to the final promoted executable.

## Regression evidence

The existing registered `engine-update-permission-policy` test was extended instead of adding a new harness target. It asserts:
- archive extraction uses a private update stage;
- the old `deleteEngineBinFolder()` / `removeFiles()` destructive pre-extraction paths are absent;
- standalone promotion uses the rollback helper rather than deleting the destination first;
- archive promotion uses rollback-capable staged directory commit;
- the final executable permission repair still targets `opts.exeBinPath`.

Before the correction the new policy failed at `archive updater extracts into live engine tree instead of staging`.

After the correction:
- the full application compiles successfully;
- `engine-update-permission-policy` passes;
- the complete restored local CTest suite passes 58/58, including Archive GUI and integration tests changed by other in-flight remediation.

## Qualification boundary

The local tree uses the retained qualification toolchain and is not a substitute for exact-head Linux and Windows CI. This branch intentionally does not include pending MDPS-AUDIT2-039 changes in `networkAccess.cpp`; integration must preserve both independent fixes.

Status remains PR-OPEN until exact-head CI and post-integration combined qualification succeed.
