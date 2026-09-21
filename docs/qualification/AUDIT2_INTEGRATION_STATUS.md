# Audit2 canonical integration status

## Canonical source of truth

- Evidence ledger: GitHub issue #2
- Coordination ledger: GitHub issue #3
- Ledger tail at this integration: `MDPS-AUDIT2-251`
- Integration base: `audit2-remediation`
- Canonical combined pull request: #213
- Canonical working branch until qualification and merge: `audit2-remediation-all-251`
- `main` remains the last qualified release line and is not rewritten by this work.

Older remediation branches are historical inputs only. The 226-248 branch is a strict subset of the combined branch; the 204-225, pre-final, and conflict-reconciliation branches are behind `audit2-remediation`.

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

Source inspection is not integration. The authoritative qualification is the **Archive Qt6 qualification** run attached to the exact PR #213 head. Both `linux` and `windows` jobs must execute real steps and conclude `success` before #213 can merge into `audit2-remediation`.

Actions artifacts and caches are ephemeral. The workflow does not upload source tarballs, Qt package caches, portable trees, or test-output archives. The final GC job deletes repository Actions artifacts and caches after every same-repository run, including failure/cancel paths. Repositories, branches, tags, commits, issues, pull requests, and GitHub Releases are never GC targets.

For a qualified `main` commit, durable identity is a small commit-specific GitHub Release manifest named `qualification-<commit>`. It records the exact commit and workflow run and is never deleted by Actions GC.

The exact successful run ID and merge commit are recorded in issue #3 when qualification completes.
