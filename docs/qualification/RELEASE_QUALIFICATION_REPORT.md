# Publication qualification identity

This file intentionally contains no hardcoded "current" commit, workflow run, artifact ID, or digest.

The previous qualification record for the pre-publication repository is preserved unchanged in substance at:
`history/RELEASE_QUALIFICATION_REPORT-4c720544.md`.

For every current candidate, GitHub Actions generates `current-release.json` beside the qualified Windows payload. That generated record is authoritative for the candidate and contains:

- repository
- Git ref
- exact source commit
- GitHub Actions run ID and attempt
- artifact name
- locally generated payload ZIP name
- SHA-256 of that exact payload ZIP

The workflow validates those fields against GitHub's runtime identity before upload. A report from another repository, commit, run, or payload is historical evidence only and must never be presented as qualification for the current candidate.
