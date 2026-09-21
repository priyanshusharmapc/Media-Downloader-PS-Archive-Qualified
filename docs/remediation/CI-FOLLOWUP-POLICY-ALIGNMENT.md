# Audit2 CI follow-up

Head before this change: `b3944732a955eacf0b20be5b6d473875f13df91c` on `audit2-remediation-all-251` (PR #213).

Workflow `35555632739` built Linux and Windows, then failed 12 CTest policy checks. Product hardening for Library containment, updater digests, you-get archive names, and single-instance stale-socket rules was already on that commit.

This follow-up:

- Aligns the 12 stale source-text assertions with the stronger live invariants.
- Fail-closes yt-dlp merger output when the quoted filename is missing, empty, or unterminated.
- Does not merge PR #213. Merge only after Linux and Windows Archive Qt6 jobs execute real steps and conclude success, and after Actions artifact storage returns to zero.
