# Testing And Qualification

## Final qualification identity

Qualification for the completed release generation is bound to:

- Product commit: `2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Run: `35679575106`
- Release: `qualification-2b658b3ff02ad49831b6c5fed14a6f8e3c8c79c2`
- Package SHA-256: `c11f21de897db6f3727fdfc8c3bdd585cd1979530864c586df824d5d8b38dd07`

Release level is **GOLD under Final Release Policy v2**.

## Mandatory results

- Linux: PASS
- Linux CTest: 205 passed
- Linux sanitizer: PASS
- Windows: PASS
- Windows CTest: 204 passed
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
- Q27 target-host media qualification: PASS

The jobs executed real product/test steps.

## Advisory results

Final Release Policy v2 makes Q25 and Q26 advisory/non-blocking:

- Q25 provider endurance: BLOCKED
- Q25 archive safety under incomplete discovery: PASS
- Q26 constrained-resource qualification: BLOCKED

Neither advisory gate is represented as PASS.

The Q25 run requested 60 minutes but stopped after approximately 7 minutes / 75 iterations when provider discovery became incomplete. MDH failed closed and disabled removal inference.

The Q26 attempt could not establish the declared symlink-capable environment on the available Windows 11 host.

## Test interpretation

Policy/static tests and executable behavioral tests are distinct evidence classes. The final CI also exercised real packaged runtime behavior, while Q27 provided exact-package target-host acceptance.

Historical `4c720544`, `bb949284`, `77caa99`, `659f6bb` and `53e9c845` evidence remains provenance and must not be silently substituted for final C_final evidence.

See `FINAL_RELEASE_POLICY_V2.md` for the controlling gate definition.
