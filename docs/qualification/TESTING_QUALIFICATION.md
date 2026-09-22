# Testing And Qualification

Qualification always applies to the exact commit named by the current commit-specific GitHub Release. The historical documented product was `bb949284c15a2ce1128e94d41a931fe407358e64`; the current operational baseline before final-candidate normalization is `53e9c8458298ba3396031f2342fd9f4807559311`. Gold remains unset until every mandatory Gold gate passes.

The latest operational baseline was qualified by Archive Qt6 push run `35652404033` on `53e9c8458298ba3396031f2342fd9f4807559311`; Linux, Windows, sanitizer, packaged runtime, release publication and verification all succeeded. Earlier `bb949284...`, `77caa99...` and `659f6bb...` records remain historical candidate records and are not silently promoted to the final candidate.

The media suite uses real FFmpeg-generated fast-start MP4/M4A fixtures. Metadata-readable truncation is rejected, corrupted canonical media is repaired by sync, valid media passes, and full stream consumption is required.

CI YouTube discovery remains advisory (`exit 0`) and is not endurance evidence. Historical `4c720544` / run `35116148963` numbers are not current qualification.

`GOLD-4-PAUSED-LOWMEM` and `GOLD-4-PAUSED-ENDURANCE` are operator gaps, not PASS. A 60-minute identity-bound endurance run and serial low-memory CTest were not executed for this product. Any unavailable provider or host-specific operation is recorded as blocked, not inferred as PASS.
