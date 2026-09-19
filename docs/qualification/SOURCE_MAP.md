# Source Map

- `src/archive/`: Archive Mode core, state, reconciliation, media verification, locks, transactions and Recovery Packages.
- `src/archive/archivesafety.h`: shared filesystem and Windows reparse safety.
- `src/archive/archivesettings.cpp`: user configuration persistence outside the package.
- `src/archive/archivetab.cpp`: GUI Archive Mode integration.
- `tests/archive-core-tests.cpp`: canonical state and identity tests.
- `tests/archive-hardening-tests.cpp`: path, reparse, state, import and recovery tests.
- `tests/archive-media-integrity-tests.cpp`: real media profile/truncation/integrity tests.
- `tests/archive-integration-tests.py`: deterministic end-to-end tests with real FFmpeg.
- `scripts/archive-local-harness.ps1`: sealed Windows target-host harness.
- `scripts/archive-endurance.ps1`: identity-bound endurance runner.
- `.github/workflows/archive-qt6.yml`: maintained build, test, sanitizer, package and evidence workflow.

The publication copy excludes internal historical qualification logs and generated IDE metadata. The qualified implementation source remains bound to commit `4c72054465697b899ede0555e48aff8a7c91c225`.
