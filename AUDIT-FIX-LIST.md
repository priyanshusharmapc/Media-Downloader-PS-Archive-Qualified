# Audit remediation fix list

Branch: `audit-remediation-20260916`

- [x] Full FFmpeg decode/integrity verification for canonical media
- [x] Preserve Archive Root path until link/reparse validation
- [x] Windows-native reparse-tag checks with Cloud Files handling
- [x] Move Archive GUI settings outside the sealed application directory
- [x] Remove inherited upstream publisher/dispatch workflows
- [x] Supersede stale RELEASE READY documentation pending fresh qualification
- [ ] Execute >=60-minute endurance qualification (runner provided)
- [ ] Redesign unknown-ID placeholder identity without mutable playlist position
- [x] Extend Windows reserved-device-name validation
- [x] Correct verify-item documentation semantics
- [x] Restore Unix executable mode bits

The source patch is prepared from publication commit `cc8bb65d8d38d482f94da1eb1edd2714b242f577`.