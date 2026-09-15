# Release Notes

## Candidate Identity

- Source commit: `ad7943b93b0458ff2764432ae67419104c43fff8`
- Branch: `luna-max-mdps-live-001`
- CI run: `34981539523`
- Portable candidate SHA-256 is recorded in the outer manifest.
- Candidate `archive-cli.exe`: `481abf073856253d81eda96c54c423ffa1be3eccd7d094abc832d249f19defd6`
- Candidate `media-downloader.exe`: `21c6be6f35020eddbcc9fd4d7f2653f377634b266edefa01379d056134d96c16`
- Candidate `archive-local-harness.ps1`: `806d828fcb87bbb1265b6b2f34fa704dd74aceade6835bb781b9f00ff807c1f2`

## Fixed

`MDPS-LIVE-001` rejected a non-redirecting Microsoft Cloud Files ancestor because the staged classifier used the wrong bit mask and signed PowerShell constants. The repaired classifier accepts only the documented Cloud Files family, including `0x9000601A`, while continuing to reject links, junctions and unknown reparse tags.

## Qualification Result

The candidate passed CI, the exact Z-root target-host harness, live three-item acceptance, package sealing, tamper rejection, Recovery Package tests, backup/restore, relocation, scale, soak, media verification and GUI startup smoke. See the qualification report for mandatory gates that remain blocked.

## Known Limitations

- Provider availability, network access and extractor behavior can change.
- A real OneDrive ancestor cannot be recreated under the mandated relocated Z workspace; the tag-family regression is synthetic and exact.
- The Windows symlink privilege test is host-dependent and was skipped in the CI CTest report; junction and linked-state/package rejection were independently exercised.
- Agent Manager resolved GPT-5.6 Luna `max` for dispatch but exposed no runtime session metadata for the requested independent worker. That routing limitation is recorded as a blocked gate, not a PASS.
