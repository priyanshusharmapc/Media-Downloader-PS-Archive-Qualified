# Archive integrity remediation: findings 033, 034, 035, 036

## Scope and status

Prepared on 2026-09-20 (Asia/Kolkata). Source remediation and executable local regressions are implemented. This record is not a claim of integrated CI qualification or whole-project completion. Issue #3 retains the live status.

Repository: `priyanshusharmapc/Media-Downloader-PS-Archive-Qualified`.
Parent integration commit: `2d66d1d42a8737b6577ab06120c41cf0daa94231`.
The immutable audit baseline and issue #2 have not been edited.

Canonical IDs follow the final numbering correction in issue #2, not every original comment heading. In particular, 034 is missing playlist state (comment 5712112891), and 035 is normalization staging leakage (comment 5712131781). The video-dimension PR labeled 035 does not cover this staging defect.

## Behavior and compatibility

### MDPS-AUDIT2-033: semantic history validation

`archivehistory.h` validates version, timestamp, event identity and event-specific field types before append, reconciliation or transaction replay. `appendHistory` validates the existing stream and uses journaled atomic replacement instead of an unchecked append. Corruption is reported without rewriting or discarding the original history.

Existing event writers remain supported, including the playlist-wide migration event with its deliberately empty item key, metadata-only recovery with no promoted media paths, CRLF/blank lines, unknown provider availability strings and historical media references that no longer exist. Unknown event/schema versions stop the writer for explicit repair or an appropriate upgrade.

### MDPS-AUDIT2-034: missing authoritative occurrence state

A missing `Playlists/<source>/items.json` is no longer automatically interpreted as an empty playlist. Registered-source scan state, retained metadata and history distinguish a first admission from loss of a previously managed file or whole directory. The check covers loads, direct saves, reconciliation and initialization.

Valid new sources and supported legacy minimal registry entries still reach their first scan. Existing occurrence-identity migration remains supported. To recover a damaged source, restore its original authoritative state from a known-good backup. Do not fabricate an empty array to suppress the error, because it would discard historical membership.

### MDPS-AUDIT2-035: normalization staging ownership

Automatic video/audio normalization uses an owned `QTemporaryDir`. Encoding failures, staged verification failures and destination collisions reclaim the attempt's staging bytes. Successful publication retains the existing non-overwriting rename behavior. Downloaded originals, existing destinations and unrelated temporary files are not deleted by this cleanup.

This is cleanup for normal scope exits and reported failures, not a guarantee against power loss or an uncatchable application kill during encoding.

### MDPS-AUDIT2-036: POSIX descendant cleanup

Archive tools start in a dedicated process group before exec, using the appropriate Qt5/Qt6 child-setup hook. Timeout and output-overflow handling terminates the owned group rather than only its leader. Group identity is captured before Qt clears the leader PID. Linux terminal zombies are distinguished from processes that can still hold files. Failure to confirm termination is reported explicitly. Windows retains its existing tree-kill mechanism.

The executable tests cover ordinary descendants that retain the tool's process group. They do not certify deliberate daemonization into another session, every application-shutdown path or uninterruptible kernel I/O. Production timeouts and output limits have not been reduced or made bypassable through a test environment setting.

## Executed regression evidence

Each new regression was observed failing before its root correction:

| Finding | Before-fix failure | Passing coverage |
| --- | --- | --- |
| 033 | `semantically corrupt event accepted: missing schema_version` | 8 valid event forms, 57 invalid semantic cases, append/scan/restart/replay byte preservation |
| 034 | `lost playlist state accepted by load, case 0` | 4 first-admission modes, 7 loss/evidence cases, restoration preserving removed membership |
| 035 | `owned staging media leaked` | 8 video/audio cases covering encoding, verification, collision and successful publication |
| 036 | `descendant survived timeout while Archive writer ownership could be released` | Live descendant writers under timeout and overflow, next-writer safety, normal exit and channel handling |

The complete restored local application built successfully in normal Debug and ASan/UBSan configurations. Final results were 58/58 CTest entries passing in each configuration. The existing Windows-only PowerShell acceptance test remained skipped on Linux. The integration entry includes the eight new normalization tests and existing real FFmpeg/FFprobe tests.

An initial whole-suite run exposed a legacy minimal-source compatibility regression. Production code was corrected and the existing test was retained unchanged. An initial sanitizer run lacked the GUI executable; the full application was then built and all 58 entries rerun successfully. Neither initial failure was reclassified as a pass.

## Qualification boundary and provenance

The container could not clone or access package servers directly. Local compilation used the retained source artifact at `b86967d004fc9273e995d1ee7206f6755fbc144a`, with the current Archive core/header and affected test inputs reconstructed from the live integration branch and blob-hash checked before applying these fixes. Other unrelated source files and local CMake inputs remained at the retained snapshot. Therefore 58/58 is local regression evidence, not a complete test count for the final remote tree.

The published Git tree inherits the full current parent tree `345a27dfc6c42b8215c5d1dbb8b121fe02d8ce2c`; it does not replace the repository with the retained snapshot. Exact-head Linux/Windows CI, semantic integration with other open PRs, and post-merge qualification remain mandatory.

Source artifact: run 35446651286, artifact 10585074950, SHA-256 `45d17abc1b25be316c23e4850f0af90225b6ac356c39490a50340e2ff472b312`.
Qt development artifact: parent repository run 34810997639, artifact 10334423637, SHA-256 `3337b8c5589f76524d9f4e4b1f0ae9998522b643ab8dc0287bfe027ae009f6d9`.
Local environment: Debian 13, GCC 14.2, Qt 6.8.2.

## Reproduction

With Qt6 development packages, CMake, Ninja, Python and FFmpeg/FFprobe available:

```sh
cmake -S . -B build -G Ninja -DBUILD_WITH_QT6=ON -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure

cmake -S . -B build-sanitized -G Ninja -DBUILD_WITH_QT6=ON -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build-sanitized --parallel 4
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  ctest --test-dir build-sanitized --output-on-failure
```

The restored toolchain additionally required its own `CMAKE_PREFIX_PATH`, library path and Qt plugin path. Those container-specific paths are not production configuration changes.

## Retained log identities

| Log | SHA-256 |
| --- | --- |
| history-before-test.log | 1f3652cb8a5474f1a2fb4cb1010fd5a93bdeac279435e2c28e82f58d474a99a0 |
| missing-before-test.log | 6bcb1c3aec7026ba1262c3bd2c2f53a69d404556ce9e1e3182ca1f18e6d3a031 |
| normalization-before-test.log | 8eb4eac9e73756b1ff635e21da292ff7caf21f2e806a5c76c9ef3d926ff6e2dc |
| process-before-test.log | f4237a4922465dfea615920436083888c7c8599ca1881afcbd82b706330a0bda |
| combined-ctest.log | a1f21df62a30474c6a00d07fbf1e81300c7064007087d3e9b49e3bd433c103ff |
| sanitizer-full-ctest.log | 2db918452cad8912cddc2303169ce1cc3a5523f191cf37bf5c622ece82f58dcf |
