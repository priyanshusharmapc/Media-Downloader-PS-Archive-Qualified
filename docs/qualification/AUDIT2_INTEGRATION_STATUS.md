# Audit2 canonical integration status

## Canonical source of truth

- Evidence ledger: GitHub issue #2
- Coordination ledger: GitHub issue #3
- Ledger tail at this integration: `MDPS-AUDIT2-251`
- Integration base: `audit2-remediation`
- Canonical combined pull request: #213 (merged)
- Canonical working branch until qualification and merge: `audit2-remediation-all-251`
- Qualified **product** commit `C_main`: `bb949284c15a2ce1128e94d41a931fe407358e64`
- `main` is not rewritten. A later docs or verifier commit on `main` is not `C_main`.

Older remediation branches are historical inputs only. The 226-248 branch is a strict subset of the combined branch; the 204-225, pre-final, and conflict-reconciliation branches are behind `audit2-remediation`.

## Product identity (gold incomplete)

The qualified product commit is `bb949284c15a2ce1128e94d41a931fe407358e64`. GitHub Release `qualification-bb949284c15a2ce1128e94d41a931fe407358e64` holds the portable ZIP (`payload_sha256=8704235825ab2b5f9a3c6fe5eadd052ec08402214f7225d3e040b94f6b0e1cfd`) from run `35613369266`. Commit `77caa99e421ce6d9476d02c3d34c927d963b2528` is a verifier-only follow-up (`GH_REPO` on the ubuntu publisher). Sibling Release `qualification-77caa99e421ce6d9476d02c3d34c927d963b2528` is not the gold product. Gold 4 (endurance and low-memory) is **paused**: `GOLD-4-PAUSED-LOWMEM` (no cmake/ninja/Qt on the L16 host) and `GOLD-4-PAUSED-ENDURANCE` (no operator PlaylistUrl/VideoUrl). This is not a complete gold-state release.

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

For the qualified product commit, durable identity is GitHub Release `qualification-bb949284c15a2ce1128e94d41a931fe407358e64`. It records the exact commit and workflow run and is never deleted by Actions GC. Do not retag or clobber it. Sibling `qualification-77caa99e421ce6d9476d02c3d34c927d963b2528` is not gold.

Issue #3 records `C_main`, `R_main`, and gold-4 paused. This is not a complete gold-state release.
