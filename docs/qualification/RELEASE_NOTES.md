# Release Notes

## Final release

- Product commit: `2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Qualification run: `35679575106`
- Release: `qualification-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Portable ZIP: `Media-Downloader-PS-Windows-Qt6-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2.zip`
- ZIP SHA-256: `c11f21de897db6f3727fdfc8c3bdd585cd1979530864c586df824d5d8b38dd07`
- Release level: **GOLD under Final Release Policy v2**
- Audit2: **CLOSED**
- Further execution required: **no**

A later documentation-only/control-plane commit does not replace this product baseline.

## Qualification summary

The final product passed Linux, Windows, Linux sanitizer, full required CTest, portable package assembly, CLI preflight, GUI launch, packaged yt-dlp/FFmpeg/FFprobe checks, real-media smoke, package immutability, release publication, tag verification, payload digest verification, Actions cleanup, and Q27 target-host media qualification.

### Advisory gates

Final Release Policy v2 reclassifies Q25 and Q26 as advisory/non-blocking.

- Q25 provider endurance: **BLOCKED / advisory**
- Q25 archive safety under incomplete discovery: **PASS**
- Q26 constrained-resource qualification: **BLOCKED / advisory**
- Q27 target-host media qualification: **PASS / mandatory**

Q25 and Q26 are not PASS.

## Major hardened areas

The release incorporates Audit2 remediation through `MDPS-AUDIT2-251`, including archive integrity, filesystem safety, transaction/recovery behavior, process lifecycle hardening, updater/component safety, native filename identity, single-instance behavior, packaged runtime integrity, and qualification provenance.

## Permanent limitations

Provider availability, network conditions, authentication/region behavior and future extractor changes remain external dependencies. A complete current-package 60-minute provider-endurance run was not achieved. The attempted Q25 run failed closed and preserved archive safety. A formal symlink-capable constrained-resource Q26 environment was not available for the final attempt.

See `FINAL_RELEASE_POLICY_V2.md` and GitHub issue #219 for the controlling final policy.
