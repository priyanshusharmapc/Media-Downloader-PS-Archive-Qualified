# MDPS-AUDIT2-019 remediation

## Defect
The Archive GUI treated persisted representation state `complete` as current integrity proof. A deleted, empty or later-modified media file could therefore remain counted and displayed as `Protected`, while details could claim the representation was present.

## Correction
The GUI now separates durable workflow state from current health. A complete representation is considered currently verified only when:
1. its stored path is still a safe Archive-relative path;
2. the path still names a non-empty regular file;
3. `verifiedAt` is a valid timestamp from a completed verifier pass; and
4. the file modification time has not advanced beyond `verifiedAt`.

This is an explicit low-cost freshness policy. It deliberately avoids re-decoding every media file during every UI refresh. Missing/unsafe/empty files render as `Missing`; later-modified or unbound verification evidence renders as `Verification Stale`; only both fresh representations allow `Protected`. The Video/Audio columns also expose the integrity label, and the details pane now says `Video integrity` / `Audio integrity` instead of inferring presence from state alone.

A storage failure that changes bytes without changing any filesystem metadata is outside this cache policy and is caught by the next explicit verifier pass. The UI no longer claims current protection after ordinary deletion, truncation or modification.

## Regression
The existing `archive-gui-settings-tests` target now seeds a complete item with fresh evidence, proves it is Protected, deletes the video and proves the row becomes Missing, restores fresh evidence, then mutates the file and advances its modification time and proves the row becomes Verification Stale. It also proves the details pane no longer reports `Video present: Yes` from persisted state.

The regression failed before the source correction with: `deleted completed media still shows Protected`.
After the correction, the complete existing GUI/settings test executable passes locally.

Final status remains PR-OPEN until exact-head Linux/Windows CI and post-integration qualification succeed.
