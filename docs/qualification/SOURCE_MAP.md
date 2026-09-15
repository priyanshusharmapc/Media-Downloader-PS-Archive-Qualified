# Source Map

## Repository

- `src/archive/`: Archive Mode core, CLI, state, discovery, media verification and Recovery Package implementation.
- `tests/archive-core-tests.cpp`: core state and identity tests.
- `tests/archive-hardening-tests.cpp`: path, state, discovery, import, recovery and linked-path tests.
- `tests/archive-integration-tests.py`: deterministic end-to-end tests with real FFmpeg and a fake downloader boundary.
- `scripts/archive-local-harness.ps1`: sealed Windows target-host harness and manifest verifier.
- `resources/archive/`: operator contract, schema and example Recovery Package.
- `docs/`: maintained repository documentation.
- `.github/workflows/archive-qt6.yml`: Windows package and Linux sanitizer workflow.

The publication copy is curated from the exact commit: internal historical `qualification/pre-harness` logs and generated IDE metadata are kept in the evidence workspace rather than published, and generic developer paths in the Flatpak helper are replaced with repository-relative examples.

## Authority

The tested executable source and its schemas/contracts are authoritative for behavior. Runtime evidence establishes what was executed. Repository documentation describes maintained behavior. External design documents are requirements and rationale, not proof of implementation.

## Repair

The qualification repair includes the target-host harness reparse classifier, its regression test, and the Windows linked-package safety test/CI privilege probe. It does not weaken application path safety or add a bypass switch. The qualified source is commit `261fcef529be7982b9048a39161941c1572b3044`.

## Licensing

The portable package includes `LICENSE.txt`. Bundled third-party components retain their own notices and licenses. Consult the repository license and bundled notices before redistribution.
