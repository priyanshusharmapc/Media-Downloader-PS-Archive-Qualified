# Final Release Policy v2

## Authority

This is the in-repository final release-policy record for the completed Media Downloader PS Archive release generation. The owner-approved control-plane decision is GitHub issue #219. Older planning artifacts are historical and superseded for final release decisions where they conflict with this policy.

## Frozen product identity

- Product baseline: `2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Qualification run: `35679575106`
- Release: `qualification-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Portable ZIP: `Media-Downloader-PS-Windows-Qt6-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2.zip`
- ZIP SHA-256: `c11f21de897db6f3727fdfc8c3bdd585cd1979530864c586df824d5d8b38dd07`
- Release level: **GOLD under Release Policy v2**
- Further execution required: **no**

A later documentation-only/control-plane commit does not supersede this product baseline. A new product candidate requires a product-affecting change such as implementation, behavioral tests, CMake/build behavior, qualification workflow behavior, dependencies, packaging behavior, or distributed package bytes.

## Mandatory gate results

- Audit2 remediation through `MDPS-AUDIT2-251`: COMPLETE
- Linux: PASS, 205 tests
- Linux sanitizer: PASS
- Windows: PASS, 204 tests
- Portable package: PASS
- CLI preflight: PASS
- GUI launch: PASS
- Packaged yt-dlp / FFmpeg / FFprobe: PASS
- Real-media smoke: PASS
- Package immutability: PASS
- Release publication: PASS
- Tag verification: PASS
- Payload digest verification: PASS
- Actions artifact/cache cleanup: PASS
- Q27 target-host media qualification: PASS / mandatory
- Known release-blocking product defects: 0

## Q25 advisory result

Q25 real-provider endurance is **ADVISORY / NON-BLOCKING**. Its factual result remains **BLOCKED**.

The C_final run requested 60 minutes and stopped after approximately 7 minutes / 75 iterations when provider discovery became incomplete. It observed 4 active items, 0 removed and 0 unavailable, then returned exit code 3.

MDH failed closed with:

`Incomplete or suspect discovery output; removal inference disabled`

Archive safety under incomplete discovery is **PASS**. Q25 itself is not PASS. Only its release-gate criticality changed.

## Q26 advisory result

Q26 constrained-resource qualification is **ADVISORY / NON-BLOCKING**. Its factual result remains **BLOCKED**.

The attempted Windows 11 Pro host had approximately 7.86 GiB RAM but lacked the required symlink capability / Developer Mode privilege. Ordinary Windows CI is not retroactively relabeled as Q26. Q26 is not PASS.

## Q27 mandatory result

Q27 remains **MANDATORY** and is **PASS**. The exact C_final release package passed package identity and SHA-256 verification, real playlist discovery with `complete=true`, exact video sync, video/audio verification, repeat sync/verification and idempotence.

## Audit2 closure

Audit2 is formally closed for this release generation.

- Tail: `MDPS-AUDIT2-251`
- Explicit retractions: `068`, `146`, `163`
- Non-retracted numeric IDs: 248
- Retained historical collision records: 2
- Legitimate non-retracted technical records under ledger accounting: 250
- Issue #2: CLOSED / completed
- Issue #3: CLOSED / completed
- Open remediation PRs: 0

Historical finding comments remain immutable provenance. Future defects belong to post-release maintenance or a new audit generation.

## Permanent limitations

The release permanently discloses that the current-package 60-minute provider endurance target was not completed, the attempted Q25 run failed closed while preserving archive safety, and a symlink-capable Q26 environment was not available. Provider availability, networking, authentication/region behavior and future yt-dlp extractor changes remain external dependencies.

## Final interpretation

`2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2` is the frozen final Gold product baseline under Release Policy v2. Q25 and Q26 remain advisory BLOCKED qualification, Q25 archive safety is PASS, Q27 is mandatory PASS, and the release generation is closed.
