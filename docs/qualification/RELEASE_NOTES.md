# Release Notes

## Candidate Identity

- Source commit: `261fcef529be7982b9048a39161941c1572b3044`
- Branch: `luna-max-mdps-live-001`
- CI run: `35000478514`
- Portable candidate SHA-256 is recorded in the outer manifest.
- Candidate `archive-cli.exe`: `8388abd9f129fca9f45e77cf60d7e73ebf596b40816016d0fab3cdeab93ad0dc`
- Candidate `media-downloader.exe`: `cb890dd5542464d2b476b7844a1a169d1059e6d187cbb9702cb6bba5f1850516`
- Candidate `archive-local-harness.ps1`: `806d828fcb87bbb1265b6b2f34fa704dd74aceade6835bb781b9f00ff807c1f2`

## Fixed

`MDPS-LIVE-001` rejected a non-redirecting Microsoft Cloud Files ancestor because the staged classifier used the wrong bit mask and signed PowerShell constants. The repaired classifier accepts only the documented Cloud Files family, including `0x9000601A`, while continuing to reject links, junctions and unknown reparse tags.

## Qualification Result

The candidate passed CI, the exact Z-root target-host harness, live three-item acceptance, package sealing, tamper rejection, Recovery Package tests, backup/restore, relocation, scale, soak, media verification and GUI startup smoke. The final qualification report records the release decision.

## Known Limitations

- Provider availability, network access and extractor behavior can change.
- A real OneDrive ancestor cannot be recreated under the mandated relocated Z workspace; the tag-family regression is synthetic and exact.
- The Windows symlink test requires Developer Mode or equivalent privilege; the authoritative CI runner enabled it and executed the real boundary test.
- Provider availability, network access and extractor behavior remain external dependencies.
