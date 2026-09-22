# Audit2 canonical integration status

## Canonical source of truth

- Evidence ledger: GitHub issue #2
- Coordination ledger: GitHub issue #3
- Ledger tail at this integration: `MDPS-AUDIT2-251`
- Integration base: `audit2-remediation`
- Canonical combined pull request: #213 (merged)
- Canonical working branch until qualification and merge: `audit2-remediation-all-251`
- Historical qualified product commit: `bb949284c15a2ce1128e94d41a931fe407358e64`
- Current operational baseline: `53e9c8458298ba3396031f2342fd9f4807559311`
- Final candidate: unresolved until pre-freeze normalization and baseline qualification complete.
- `main` is not rewritten. Every later source candidate has its own commit-specific qualification release.

Older remediation branches are historical inputs only. The 226-248 branch is a strict subset of the combined branch; the 204-225, pre-final, and conflict-reconciliation branches are behind `audit2-remediation`.

## Product identity (gold incomplete)

The current product identity is resolved from the latest commit-specific GitHub Release and its `current-release.json`. Historical product `bb949284...` and operational baseline `53e9c845...` remain explicitly distinguished. Gold is unset; endurance, constrained-resource, target-host and final-candidate qualification remain separate gate states.

## Final integration hardening

The combined branch preserves prior audit2 remediation and adds the final cross-cutting fixes required before qualification:

1. POSIX Library destructive operations resolve every path component descriptor-relatively using `openat` and `O_NOFOLLOW`.
2. Selected deletion checks cancellation before resolution and immediately before mutation.
3. Delete All confirmation captures and passes the exact native directory identity shown to the user.
4. Component SHA-256 verification normalizes the explicit `sha256:` prefix at the download trust boundary.
5. QuickJS and svtplay-dl update actions are disabled until their upstream metadata can provide trusted SHA-256 values.
6. Archive-folder normalization receives the original release asset filename rather than the randomized private download filename.
7. Unix single-instance startup keeps the ownership lock for the primary lifetime. Socket cleanup requires stale-class failure, successful lock acquisition, and a verification probe.
8. Policy files are valid Python and qualification compiles the complete test tree before build/test execution.
9. Runtime POSIX regression coverage includes invalid-byte display collisions, confirmation-bound Delete All, cancel-before-worker, and intermediate symlink substitution.

## Qualification and Actions storage policy

Source inspection is not integration. Product qualification for `C_main` is Archive Qt6 push run `35613369266` (`R_main`) with real Linux and Windows steps. That is not a runner-allocation or empty-step record.

Actions artifacts and caches are ephemeral. The workflow does not upload source tarballs, Qt package caches, portable trees, or test-output archives. The final GC job deletes repository Actions artifacts and caches after every same-repository run, including failure/cancel paths. Repositories, branches, tags, commits, issues, pull requests, and GitHub Releases are never GC targets.

For each qualified product candidate, durable identity is its own GitHub Release `qualification-<full-source-commit>`. Releases record the exact commit and workflow run and are never deleted by Actions GC. Existing releases must not be retagged or clobbered.

Issue #3 records `C_main`, `R_main`, and gold-4 paused. This is not a complete gold-state release.
