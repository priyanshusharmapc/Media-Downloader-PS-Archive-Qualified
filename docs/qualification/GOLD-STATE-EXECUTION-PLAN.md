# GOLD-STATE EXECUTION PLAN

Document status: planning artifact only. Not qualification evidence. Not a release claim.
Author role: Grok 4.6 High (architect / planner / orchestrator).
Executor role after this plan is accepted: Luna Max 5.6 under Grok review gates.
Critique role during this planning cycle only: Terra High.
Post-plan execution models: Grok 4.6 High and Luna Max 5.6 only.
Date of diagnosis: 2026-09-21.
Facts in §0.1–0.2 re-verified 2026-09-21T11:45Z against GitHub. They expire at the next push.
GitHub is source of truth. Chat memory is not.
Critique cycle: five Grok passes + Terra High 1 + Terra High 2 incorporated.
**FREEZE ACCEPTED.** G0 may open after this header: L1, optionally L7+L8. Not L2. Not L5. Do not spawn Luna from the freeze itself.

## L17 status note (planning artifact only; not qualification evidence)

The qualified product commit is `bb949284c15a2ce1128e94d41a931fe407358e64`. GitHub Release `qualification-bb949284c15a2ce1128e94d41a931fe407358e64` holds the portable ZIP (`payload_sha256=8704235825ab2b5f9a3c6fe5eadd052ec08402214f7225d3e040b94f6b0e1cfd`) from run `35613369266`. Commit `77caa99e421ce6d9476d02c3d34c927d963b2528` is a verifier-only follow-up (`GH_REPO` on the ubuntu publisher). Sibling Release `qualification-77caa99e421ce6d9476d02c3d34c927d963b2528` is not the gold product. Gold 4 (endurance and low-memory) is **paused**: `GOLD-4-PAUSED-LOWMEM` (no cmake/ninja/Qt on the L16 host) and `GOLD-4-PAUSED-ENDURANCE` (no operator PlaylistUrl/VideoUrl). This is not a complete gold-state release. L19 remains closed. Do not retag `qualification-bb949284c15a2ce1128e94d41a931fe407358e64`.

This plan is the only authorized execution map from the current HEAD to a publicly published, reproducible, qualified release of Media Downloader PS. Execution does not stop at a green PR.

---

## 0. Control plane

### 0.1 Workspace and identity (re-verified 2026-09-21T11:45Z)

| Item | Value |
|---|---|
| Workspace | `Z:\KILOCODE WORK\Media Downloader Harness\MDH\qualified-pr213` |
| Luna host OS / shell | Windows (`win32`) / Windows PowerShell 5.1. Linux workflow commands are **not** local acceptance. |
| Repository | https://github.com/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified |
| Working branch | `audit2-remediation-all-251` |
| HEAD | `bebe42d480db763441f5a22a7c3c7c456434a87d` |
| Working tree | clean except untracked `docs/qualification/GOLD-STATE-EXECUTION-PLAN.md`, `docs/qualification/TERRA-HIGH-1-CRITIQUE.md`, `docs/qualification/TERRA-HIGH-2-CRITIQUE.md`, and `tests/__pycache__/` |
| PR | #213 OPEN, MERGEABLE, `mergeStateStatus=UNSTABLE` |
| PR URL | https://github.com/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified/pull/213 |
| PR base | `audit2-remediation` @ `ca0ffd1408c634dde6b4d8d7569e32a32f3eb231` |
| PR head | `audit2-remediation-all-251` @ `bebe42d480db763441f5a22a7c3c7c456434a87d` |
| `origin/main` | `8a62c9b2c5d63f4f9c1cfa6373884e37bf1348c1` |
| Commits PR head is ahead of `audit2-remediation` | 108 |
| Repo merge buttons | `allow_merge_commit=true`, `allow_squash_merge=true`, `allow_rebase_merge=true`, `delete_branch_on_merge=false` |
| Issue #2 | immutable discovery ledger, tail `MDPS-AUDIT2-251` |
| Issue #3 | living coordination ledger. Latest comment 2026-09-21T01:10:34Z still names head `9c1caeb1c1dbabce7dfab4e9784547e26b661c27` (stale vs `bebe42d`). No CI-truth update for run `35586304364` yet. |
| Product version in CMake | `5.6.6` |
| GUI executable CMake form | `add_executable(media-downloader WIN32 ...)` on Windows — no console, stdout redirect is not a reliable log channel |
| Canonical working branch until qualification and merge | `audit2-remediation-all-251` |
| `main` | last qualified release line; never rewritten |

### 0.2 Latest PR CI (GitHub source of truth)

| Item | Value |
|---|---|
| Workflow | Archive Qt6 qualification |
| Run | `35586304364` |
| URL | https://github.com/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified/actions/runs/35586304364 |
| Event | `pull_request` |
| Head SHA | `bebe42d480db763441f5a22a7c3c7c456434a87d` |
| Conclusion | **failure** |
| linux | FAIL, job `106290346908` |
| windows | FAIL, job `106290346708` |
| publish durable qualification evidence | SKIPPED (not a `push` to `main`) |
| purge Actions artifact and cache storage | SUCCESS |

This is executable CI that ran real product/test steps and failed. It is not a runner-allocation blocker. Docs and issue #3 that still cite empty-step / `steps: []` / `runner_id: 0` are stale.

### 0.3 Role boundary

- Grok plans architecture, sequence, acceptance criteria, and Luna packets.
- Luna executes one packet at a time after the matching Grok gate.
- Grok inspects Luna's work, then issues the next packet or a corrected retry packet.
- Terra High critiques this plan during the planning cycle only. Terra is not used during later execution.
- No implementation, commit, merge, push, or worker spawn is authorized by this document itself.
- Luna does not merge, tag, or publish unless the packet explicitly says so and Grok has opened that gate.

### 0.4 Global forbidden actions

Never:

- weaken, skip, delete, or rewrite policy tests merely to obtain green CI
- claim qualification from local tests alone
- merge #213 until Linux and Windows Archive Qt6 jobs execute real steps and succeed on the exact head being merged
- merge #213 before L5 and L6 are on that same head (`SHA_pr`). G1b green smoke/sanitizer is not enough.
- merge to `main` until `audit2-remediation` is verified by the same workflow on the post-merge integration commit
- delete repositories, branches, tags, commits, issues, PRs, or GitHub Releases as cleanup
- invent hashes, run IDs, or qualification claims
- treat docs or chat as current CI truth
- silently ignore a check that cannot run
- use `QT_QPA_PLATFORM=offscreen` or `continue-on-error` to make Portable GUI launch smoke pass
- reduce the 5-second GUI liveness assertion
- add `actions/upload-artifact@` at all, including a "named identity transfer" exception. That would require deleting or hollowing `assert "actions/upload-artifact@" not in workflow` in `tests/ci-supply-chain-policy-tests.py`, `tests/audit2-final-integration-policy-tests.py`, and `scripts/validate-qualification-publication.py`. That is a policy weaken. Forbidden.
- edit `scripts/validate-qualification-publication.py` so PowerShell `ConvertTo-Json` can replace the grepped Python compact-JSON snippet
- add `windows.needs: [linux]` in L1, L2, L3, or L4
- `gh release create` from `pull_request` or `workflow_dispatch`
- hash an inner tool zip, a re-zipped extract, or anything other than the Release payload ZIP bytes as `payload_sha256`
- combine L1 and L2 into one commit, one push, or two concurrent editors of `.github/workflows/archive-qt6.yml`
- treat endurance or low-memory `UNAVAILABLE` as gold-state PASS
- treat `origin/main` after a docs-follow-up commit as the qualified product commit
- run Linux `test -x` / `python3` / `rg` on the Windows Luna host as if that were sanitizer qualification
- rewrite issue #2 finding comments
- force-push `main` or `audit2-remediation`
- squash-merge or rebase-merge #213 or the main PR. Repo allows all three buttons. Luna must pass `--merge` and prove two parents.
- stop at mergeable PR, local test pass, worker "done", a build that never ran on GitHub, green Linux with failed Windows, successful branch not on `main`, release artifact hash not tied to the qualified commit, or a red/skipped/cancelled/zero-step workflow

### 0.5 Source-of-truth hierarchy

1. GitHub refs, PR #213, workflow run pages, and job logs
2. Issue #2 finding comments (immutable)
3. Issue #3 coordination comments (living, may be stale; verify against 1)
4. Source and workflow files at the commit under test
5. This plan (planning artifact; not qualification evidence; SHAs in §0 expire)
6. Chat memory (never authoritative)

### 0.6 Fact expiry and Luna host

Before every packet Luna must re-run:

```powershell
git fetch origin
git rev-parse HEAD
git rev-parse origin/audit2-remediation-all-251
git status --porcelain
gh pr view 213 --json state,mergeable,mergeStateStatus,headRefOid,baseRefOid
```

If `headRefOid` ≠ this plan's HEAD table, the table is stale. Use GitHub. Do not invent a new plan SHA.

Luna's shell is Windows PowerShell. Workflow snippets that say `test -x`, `python3`, `set -euo pipefail`, or `rg` execute **on GitHub runners**, not as Luna local acceptance unless the packet gives a PowerShell equivalent.

This plan file stays **untracked until G0**. After G0, Packet L7 commits it, `TERRA-HIGH-1-CRITIQUE.md`, and `TERRA-HIGH-2-CRITIQUE.md` as planning artifacts, not qualification evidence. Do not commit them in L1–L6. Never treat them as `C_main` identity.

---

## 1. Current-state diagnosis vs gold state

### 1.1 Gold state (verbatim stopping condition)

Execution does not stop until all of the following are true:

1. Architecture
- All required remediation is integrated.
- PR #213 or successor is merged into `audit2-remediation`.
- Verified integration is merged into `main`.
- `main` contains everything required; no side branch is needed for completeness.

2. Source quality
- Known filesystem, updater, single-instance, cancellation, native identity, and yt-dlp defects are fixed.
- No known critical/high findings remain open.
- No policy tests weakened or deleted merely to obtain green CI.
- All Python policy tests compile and pass.
- Product is free of release-blocking bugs; remaining advisory gaps are explicitly documented, never silently ignored.

3. Build qualification
- Linux GitHub Actions runs real steps and succeeds.
- Windows GitHub Actions runs real steps and succeeds.
- Qt6 product builds succeed on both.
- Full test suites pass.
- Sanitizer, package, portable, recovery, and relevant runtime tests pass.

4. Extended release qualification
- Endurance testing passes.
- Low-memory testing passes.
- Portable layout and bundled tools are verified.
- Target-host/media acceptance checks pass where applicable.
- Any advisory test that cannot run in CI is executed through the local harness or explicitly documented as unavailable.

5. Release publication
- A release commit on `main` is recorded.
- A Git tag or GitHub Release identifies that exact commit.
- Portable release files have recorded SHA-256 hashes.
- Qualification evidence is committed or durably attached.
- Documentation explains what was fixed, how it was tested, and which commit is authoritative.

6. Operational hygiene
- Actions artifact retention and garbage collection work.
- Temporary artifacts are removed after each completed run.
- No repository, branch, commit, tag, or release is deleted as cleanup.
- Public repository contains no newly introduced secrets.

### 1.2 What is already true

| Gold item | Current evidence | Status |
|---|---|---|
| Remediation source through `MDPS-AUDIT2-251` exists on one branch | PR #213 head `bebe42d`; issue #3 unified-remediation comments; 108 commits ahead of `audit2-remediation` | Source-complete on the working branch, not integrated |
| PR #213 exists and is mergeable | GitHub `MERGEABLE` / `UNSTABLE` because checks failed | Cannot merge |
| Linux job executes real steps | Run `35586304364` linux: toolchain, checkout, `compileall`, publication validator, CMake, build, CTest, sanitizer configure, sanitizer build, sanitizer CTest | Real steps; sanitizer CTest failed |
| Windows job executes real steps | Same run windows: checkout, Python, Qt6/MinGW, symlink probe, configure, build, portable assemble, full CTest, packaged preflight, GUI smoke | Real steps; GUI smoke failed |
| Python policy suite compiles | Both jobs: Compile Python policy suite SUCCESS | Pass |
| Unsanitized Linux CTest | linux step "Test" SUCCESS | Pass, including UNIX native-filename target in the default build tree |
| Windows unsanitized CTest | windows step "Full regression and real-media integration tests" SUCCESS | Pass |
| Windows portable assembly | "Assemble self-contained portable candidate" SUCCESS | Pass as a packaging step; not yet launch-qualified |
| Packaged Archive preflight | SUCCESS | Pass |
| Artifact GC | "purge Actions artifact and cache storage" SUCCESS | Pass on this PR run |
| `main` not rewritten | `origin/main` still `8a62c9b` | Pass |
| Issue #2 immutable | Frozen findings through 251; do not edit | Pass as ledger |
| No `actions/upload-artifact@` | workflow + `tests/ci-supply-chain-policy-tests.py` + `tests/audit2-final-integration-policy-tests.py` | Pass as current contract; creates a later gold-state gap for portable hashes |

### 1.3 Gap matrix (gold vs now)

| Gold requirement | Current gap | Class | Blocks |
|---|---|---|---|
| Linux sanitizer suite passes | CTest 203/204 `library-posix-native-filename` **Not Run**; executable `build-sanitized/library-posix-native-filename-tests` missing | Internal CI/source | #213 merge |
| Windows portable GUI smoke | `media-downloader.exe` started then exited within 5s with code 1 | Internal product or packaging or runner GUI environment | #213 merge; also skips Windows seal/hash/runtime-e2e |
| Packaged yt-dlp/FFmpeg/FFprobe e2e | Skipped because GUI smoke failed | Consequence | Windows qualification |
| Seal identity / `SHA256SUMS.txt` / `current-release.json` | Skipped on this run | Consequence | Durable hashes |
| GUI settings preserve package seal | Skipped | Consequence | Windows qualification |
| Publish durable qualification evidence | Skipped; job only runs on `push` to `main` | Expected at this phase | Release publication |
| #213 merged into `audit2-remediation` | Open, checks red | Process | Integration |
| Verified `audit2-remediation` on GitHub | Base `ca0ffd1` is not the 251 combined tree; post-merge commit does not exist | Process | `main` merge |
| Combined tree on `main` | `main` is `8a62c9b`, last qualified line, missing audit2-251 | Process | Release |
| Known defects fixed **and verified** | Source claims 041/137/172/226-251; verification blocked by red CI | Verification | INTEGRATED status |
| No known critical/high open | Discovery ledger still has High findings; remediation is on the working branch, not on `main`. Treat as **remediated in source, not INTEGRATED** until combined CI and merge | Ledger | FIXED/INTEGRATED claims |
| Policy tests not weakened | Must remain true through all packets | Constraint | All work |
| Endurance | `scripts/archive-endurance.ps1` exists; no run bound to a current qualified package | Extended | Gold 4 |
| Low-memory | Procedure defined in L16a; not yet executed. Historical `4c720544` evidence is not current. | Extended | Gold 4 |
| Target-host harness | `scripts/archive-local-harness.ps1` requires `ExpectedArtifactId` (Actions artifact ID) and an externally anchored ZIP; workflow no longer uploads Actions artifacts | Architecture | Gold 4 and 5 |
| Portable SHA-256 durably attached | Windows job can generate them only after GUI smoke; publish job cannot see the Windows workspace; validator forbids `upload-artifact` | Architecture | Gold 5 |
| Docs explain the qualified commit | `docs/qualification/README.md`, `RELEASE_NOTES.md`, `TESTING_QUALIFICATION.md` still cite historical `4c720544` / run `35116148963` as if current. Other docs still cite runner-allocation blockers | Docs | Gold 5 |
| Issue #3 current | Last qualification comment records empty-step run `35549967259` and head `9c1caeb` | Ledger | Coordination |
| Evidence policy vs workflow | `EVIDENCE_POLICY.md` / `RELEASE_QUALIFICATION_REPORT.md` require `qualification-evidence-<commit>.zip` plus manifest. Workflow publishes only compact `qualification-${GITHUB_SHA}.json`. Validator requires **both** the compact JSON in the workflow **and** zip language in the current report | Architecture / docs | Gold 5 |
| Advisory YouTube in CI | `continue-on-error: true` and always `exit 0`; local harness is authoritative | Advisory; must remain documented, not treated as PASS | Gold 4 |

### 1.4 False-done signals already visible

Do not treat any of the following as gold:

- PR #213 `MERGEABLE`
- unsanitized Linux CTest PASS
- Windows CTest PASS
- "Source remediation complete through 251"
- historical `4c720544` qualification
- issue #3 comment that CI is a runner-allocation blocker
- `docs/qualification/AUDIT2_INTEGRATION_STATUS.md` saying source inspection is not integration (true) while other docs still talk as if qualification were only waiting on runners
- artifact-gc SUCCESS
- Luna or any worker saying a packet is done without GitHub job logs
- a later green Linux with still-red Windows
- L1+L2 combined push where one job is green
- rewriting `actions/upload-artifact@` policy asserts as a "contract upgrade"
- endurance/low-memory marked UNAVAILABLE and then treated as gold 4
- `origin/main` after D2 docs commit treated as `C_main`
- local Windows skip of `library-posix-native-filename` because the host is not UNIX
- squash/rebase merge of #213 that is green but not a two-parent merge commit
- GUI smoke that "passes" because `Start-Process -RedirectStandardOutput` threw and the throw was skipped
- installing a Linux-style virtual display / Xvfb on Windows, or `QT_QPA_PLATFORM=offscreen`, as the smoke success path
- ubuntu `publish-qualification-evidence` `exit 0` because tag exists, without a ZIP asset
- polling sibling jobs as a substitute for `needs:`
- `git revert` of a squash of the 108-commit integration
- L16b/L16c run with `<angle-bracket>` tokens still in the command line
- merging #213 before L5/L6 YAML is on `SHA_pr`
- treating `qualification-C_docs` as the gold Release
- `cmp` of a newly built `Compress-Archive` ZIP against the first Release ZIP as a retry requirement
- invoking `archive-endurance.ps1` from inside the portable ZIP
- `QT_QPA_PLATFORM=offscreen` or “GUI session setup” as L3

---

## 2. Immediate CI failure diagnosis plan

Two independent **failures**. Diagnose both. Do not merge until **both** jobs succeed on the same head. Luna **packets** L1 and L2 are serialized only because they edit the same YAML file, not because the defects share a root cause.

### 2.1 Linux sanitizer missing executable

#### Observed

From job `106290346908`, step "Sanitizer regression and integration tests":

- 202 of 204 tests passed, including all Python policy tests and `archive-integration`
- Test 203 `library-posix-native-filename` **Not Run**
- CTest: `Unable to find executable: .../build-sanitized/library-posix-native-filename-tests`
- Exit code 8

Unsanitized linux step "Test" succeeded. That tree is configured with `-DBUILD_TESTING=ON` and built with `cmake --build build --parallel 2` (no target filter). Therefore the UNIX target **is registered and was compiled in the normal tree**.

#### Root cause (source-confirmed, not a guess about CMake UNIX gating)

`CMakeLists.txt` lines 564-575 (UNIX native-filename target; 536 is `archive-fake-tool`):

```text
if(BUILD_TESTING)
    ...
    if(UNIX)
        add_executable(library-posix-native-filename-tests
            tests/library-posix-native-filename-tests.cpp
            src/directoryEntries.cpp)
        ...
        add_test(NAME library-posix-native-filename COMMAND library-posix-native-filename-tests)
    endif()
endif()
```

Sanitizer configure already passes `-DBUILD_TESTING=ON` and runs in a Linux container, so CMake **does** add the test. That is why CTest knows the name.

Sanitizer **build** is filtered:

```yaml
cmake --build build-sanitized --target media-downloader archive-core-tests archive-hardening-tests archive-media-integrity-tests archive-gui-settings-tests archive-cli archive-fake-tool --parallel 2
```

`library-posix-native-filename-tests` is absent from that list. CTest then runs the full test set from the sanitizer build directory and finds a registered test with no binary.

This is the same class of bug documented historically in `docs/remediation/033-036-archive-integrity.md` ("an initial sanitizer run lacked the GUI executable"). The correct repair is to build the missing target, not to skip the test.

#### Diagnosis steps Luna must still perform (confirm, then fix)

1. Re-read `.github/workflows/archive-qt6.yml` sanitizer configure/build/test steps at HEAD.
2. Re-read `CMakeLists.txt` around the UNIX native-filename target.
3. Confirm `tests/library-posix-native-filename-tests.cpp` is a real MDPS-AUDIT2-251 regression (invalid UTF-8 filename, display-colliding sibling, queued snapshot, native delete). It is. Do not delete or `#ifdef` it out.
4. Optional local Linux confirmation, if a Linux builder exists:

```sh
cmake -S . -B build-sanitized -G Ninja -DBUILD_WITH_QT6=ON -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-sanitized --parallel 2
test -x build-sanitized/library-posix-native-filename-tests
ctest --test-dir build-sanitized -R library-posix-native-filename --output-on-failure
```

If no Linux builder is available, the next GitHub linux job is the verification. Local Windows cannot compile this UNIX target; that is not a skip of the GitHub gate.

#### Required fix class

Preferred (prevents the next omitted-target failure):

```yaml
- name: Build instrumented Archive executables
  run: cmake --build build-sanitized --parallel 2
- name: Prove sanitizer native-filename binary exists
  run: test -x build-sanitized/library-posix-native-filename-tests
```

Acceptable minimum: add `library-posix-native-filename-tests` to the existing `--target` list **and** keep the `test -x` proof step.

After the binary exists, CTest must **run** test 203, not mark it Not Run. If it then fails under ASan/UBSan, that is a new product defect. Fix the product or the test implementation. Do not skip, `SKIP_RETURN_CODE`, or remove the test.

#### Acceptance

- Next linux job on the fixed HEAD: sanitizer step conclusion `success`
- CTest summary does not list `library-posix-native-filename (Not Run)`
- Job log contains a **Passed** (or a real Failed that is then fixed) line for test 203
- `test -x build-sanitized/library-posix-native-filename-tests` succeeded
- No policy test changed

#### Forbidden

- `set_tests_properties(library-posix-native-filename PROPERTIES DISABLED TRUE)`
- moving the target outside `if(BUILD_TESTING)` and then not building it
- excluding the test from sanitizer CTest via `-E`
- claiming the unsanitized PASS substitutes for sanitizer

### 2.2 Windows portable GUI exit code 1

#### Observed

From job `106290346708`, step "Portable GUI launch smoke":

```text
$process = Start-Process -FilePath "$out/media-downloader.exe" -WorkingDirectory $out -PassThru
Start-Sleep -Seconds 5
$process.Refresh()
if ($process.HasExited) { throw "Portable GUI exited during smoke test with code $($process.ExitCode)" }
```

Result: `Portable GUI exited during smoke test with code 1`.

No stdout/stderr capture. No `QT_DEBUG_PLUGINS`. No crash-dump extract in the failed-log. Later Windows steps were skipped:

- Packaged yt-dlp FFmpeg FFprobe end-to-end runtime smoke
- YouTube discovery advisory evidence
- Seal qualification identity and manifest
- Generate and validate exact release identity
- GUI settings preserve package seal

So this single failure currently also blocks portable hashes and runtime smoke.

Prior Windows steps that **did** pass: Qt6/MinGW install, configure, Release build, windeployqt assembly, pinned yt-dlp/deno/ffmpeg download+SHA-256, full CTest with `QT_QPA_PLATFORM=offscreen`, packaged `archive-cli.exe preflight`.

The GUI smoke intentionally does **not** set `QT_QPA_PLATFORM=offscreen`. That is correct. Do not weaken it.

#### Likely cause classes (ordered)

These are hypotheses for diagnosis. Luna must capture evidence before choosing a fix.

1. **Missing Qt platform plugin or runtime DLL in the portable tree** after `windeployqt`. Typical Qt message: "This application failed to start because no Qt platform plugin could be initialized." Exit 1, no crash dump. Check `$out/platforms/qwindows.dll`, `Qt6Core.dll`, `Qt6Gui.dll`, `Qt6Widgets.dll`, MinGW `libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`.
2. **QApplication / QPA failure**. `media-downloader` is linked `WIN32` (no console). GitHub `windows-2022` provides a session; there is **no Xvfb equivalent** to install. Do not tell Luna to add a "virtual display". If `qwindows.dll` is present and Qt still cannot initialize, capture the Qt plugin log. `QT_QPA_PLATFORM=offscreen` as the smoke success path **is** a weaken and is forbidden. Changing the smoke to a console-subsystem rebuild is also forbidden.
3. **Single-instance secondary exit**. `src/main.cpp` uses `utils::app::runOneInstance`. CTest already ran `media-downloader` test hooks and `archive-gui-settings-tests`. If a lock/socket in the user config path is left behind, the portable GUI may decide it is secondary and exit. Capture `$env:LOCALAPPDATA` / enginePaths socket path, `Get-Process media-downloader*`, and lock files. A secondary-instance clean exit might be 0 rather than 1; still check.
4. **Startup fatal in product code** (theme bootstrap, updater startup lock, settings, Archive Root init). `main.cpp` itself mostly returns 0 for version/test/update paths. Exit 1 is more likely Qt fatal, uncaught, or `QApplication` abort. Enable Qt logging.
5. **Hard crash**. Check `C:\Users\runneradmin\AppData\Local\CrashDumps`, WER, and `media-downloader.exe` exit after WerFault. User noted no dump in the failed-log extract; that means the smoke step did not collect dumps, not that none existed.

#### Diagnosis instrumentation (Packet L2)

Keep the throw. Add evidence around it.

Required captures before the throw:

- `Get-ChildItem $out -Recurse | Select-Object FullName,Length`
- existence of `platforms/qwindows.dll` and core Qt/MinGW DLLs
- `Get-Process` matching `media-downloader` before start
- set `QT_DEBUG_PLUGINS=1`, `QT_LOGGING_RULES='qt.qpa.*=true'`, `QT_ASSUME_STDERR_HAS_CONSOLE=1` **before** start
- launch with `Start-Process -PassThru` **without** `-RedirectStandardOutput` / `-RedirectStandardError` on the first attempt. `WIN32` GUI binaries commonly make those redirects throw or attach empty pipes; a redirect throw must **not** skip the liveness assertion
- after 5s: print `HasExited` / `ExitCode`; if log files exist, print them; list `$env:LOCALAPPDATA\CrashDumps` if the directory exists
- do **not** add a second `-s` launch inside the same step; it would change process-lifetime behavior and can mask single-instance bugs

The default smoke must still launch with no extra flags and fail if the process has exited.

#### Fix classes after evidence

| Evidence | Fix | Forbidden substitute |
|---|---|---|
| Missing `qwindows.dll` or MinGW/Qt DLLs | Repair portable assembly / windeployqt arguments / copy compiler runtime | Skip smoke |
| Plugin present, Qt log says platform init failed | Fix packaging (plugin path, `qt.conf`, MinGW DLLs) or product init. Keep 5s liveness on default `media-downloader.exe`. Do not add Xvfb/offscreen. | `QT_QPA_PLATFORM=offscreen` as pass path |
| Secondary instance / leftover lock | Isolate smoke config dir; or fix lock lifetime in product; do not disable single-instance for CI | `-s` as the only smoke |
| Product abort in theme/updater/archive init | Fix product with regression; keep smoke | Comment out init |
| Crash dump | Fix crash; add regression if feasible | continue-on-error |

#### Acceptance

- Next windows job: Portable GUI launch smoke SUCCESS
- Process still alive after 5 seconds on the default launch (no offscreen)
- Subsequent steps run: runtime e2e, seal, `current-release.json`, GUI settings seal
- Smoke test assertion text still throws if `HasExited`

#### PASS path (gold 3 Windows smoke is reachable)

`windows-2022` provides a logon session. CTest already passed on this runner with widgets offscreen. The portable smoke is a **packaging/init** gate on a session that exists, not a headless impossibility.

L2 must print (not change pass/fail):

```powershell
quser 2>&1 | Write-Host
Get-Process winlogon -ErrorAction SilentlyContinue | Format-Table Id,ProcessName | Out-String | Write-Host
```

Ordered L3 path after L2 logs:

1. Missing `qwindows.dll` / MinGW / Qt DLLs → fix portable assemble / `windeployqt` / `qt.conf`. Keep smoke. **This is the expected gold-3 path.**
2. All required files `PRESENT`, crash dump exists → product crash; Grok names source files. Still a reachable fix.
3. All required files `PRESENT`, `Get-Process winlogon` returns a process, no dump, exit 1 → product init/single-instance/QApplication. Grok names files. Still reachable. Empty WIN32 stderr is expected. `quser` failure (`No User exists for *`) is diagnostic noise, **not** a pause.
4. **`GOLD-3-PAUSED-NO-SESSION` only if** required DLLs are `PRESENT` **and** `Get-Process winlogon` returns nothing. `quser` failure is **not** this state. Return to Grok. Do not pre-authorize offscreen.

Gold 3 is reachable via (1)–(3). (4) is an external pause, not a plan hole that authorizes a weaken.

### 2.3 Shared rule for both failures

GitHub job logs of the **next** run are the only acceptance evidence. Local reproduction is allowed and useful; it is not qualification.

---

## 3. Ordered execution phases

Phase numbers are gates. A later phase must not start because a worker is bored.

### Phase A — Plan freeze

- This document is the plan.
- Terra High may critique it during the planning cycle.
- Grok accepts or revises the plan.
- No product commits yet.

### Phase B — Unblock PR CI (internal)

L1 and L2 both edit `.github/workflows/archive-qt6.yml`. They are **serialized**, never combined. Cost: two PR CI cycles (~linux 10 min + windows 15 min, twice) instead of one. Coupling remaining: L2 must not revert the L1 sanitizer hunk (`Select-String` proof). Correctness beats recovering that wall time.

1. Packet L1 only: sanitizer target completeness. Push. Wait for the linux job on that SHA.
2. Grok Gate G1-linux: sanitizer binary exists and test 203 ran (pass or real fail).
3. Packet L2 on the L1 HEAD: Windows GUI smoke diagnosis (fail-closed, more logs). Separate commit. Push.
4. Grok Gate G1-windows: logs sufficient or smoke already green.
5. Packet L3: Windows GUI root-cause fix based on L2 evidence.
6. Packet L4: only if sanitizer test 203 now **runs and fails** under ASan/UBSan.
7. Both linux and windows must execute real steps and succeed on the **same** head before G3.

### Phase C — Evidence-contract architecture (internal, Grok-designed, Luna implements after Gate G2)

Must be designed before `main` publication, may land on the working branch before merge if it does not weaken tests:

- Durable portable ZIP + `SHA256SUMS.txt` + `current-release.json` must reach GitHub Release `qualification-<commit>` for the **main** commit.
- Target-host harness must bind to that durable ZIP digest, not to a deleted Actions artifact ID.
- Policy tests are updated to the new contract, never deleted.

This can overlap Phase B design work but must not delay B's CI fixes.

### Phase D — Docs and ledger truth (internal)

- Stop citing runner-allocation / empty-step CI as current.
- Do **not** write a new `4c720544`-style "this candidate is qualified" claim.
- Issue #3 append-only update with run `35586304364` as the current executable failure.
- Issue #2 is not edited.

### Phase E — Merge #213 into `audit2-remediation`

Only after Phase B GitHub green on the exact head.

### Phase F — Verify integration branch

`audit2-remediation` push must itself run Archive Qt6 and both jobs succeed on the integration commit.

### Phase G — PR integration → `main`

New PR. Not #213. Same workflow must succeed on that PR head.

### Phase H — Merge to `main`

Creates the release-line commit `C_main`. Never rebase `main`.

### Phase I — Qualify `C_main`

`push` to `main` runs Archive Qt6 against `C_main`. Both jobs real-step success. Publish job runs.

### Phase J — Durable publication

GitHub Release `qualification-C_main` exists, bound to `C_main` and the successful run ID, with portable hashes attached.

### Phase K — Extended qualification

Endurance, low-memory, portable layout, target-host/media. Advisory gaps documented.

### Phase L — Bind documentation and ledgers to `C_main`

Replace historical-as-current claims. Record INTEGRATED/FIXED only now.

### Phase M — Gold-state audit by Grok

Walk section 1.1 verbatim against GitHub. Stop only when every line is true.

---

## 4. Architecture of remaining work

### 4.1 Source / tests

Working-branch source already contains audit2 remediation through 251, including POSIX native filename identity (`src/directoryEntries.cpp` + `tests/library-posix-native-filename-tests.cpp`), updater permission/extraction bounds, Library population generations, yt-dlp nightly Windows ARM64, theme atomic self-heal, and policy tests.

Remaining source work is whatever Phase B diagnosis proves:

- sanitizer build graph (CI) and any ASan defect that appears once the binary exists
- Windows portable launch (packaging and/or product)
- harness identity rebind (script + integration test) after Grok contract approval

Do not open a new remediation branch for 226-251. The combined branch is canonical.

If issue #2 later gains `MDPS-AUDIT2-252+` before gold, stop merge-to-main planning and return to Grok. Discovery may continue; it does not silently expand this PR.

### 4.2 CI

File: `.github/workflows/archive-qt6.yml`

Required CI architecture after Phase B:

- linux: default build + full CTest; sanitizer configure with `BUILD_TESTING=ON`; **full** sanitizer build; proof that UNIX extra binaries exist; full sanitizer CTest with ASan/UBSan halt-on-error
- windows: Qt6 MinGW; symlink probe; Release build; portable assemble with pinned hashed tools; full CTest; preflight; **GUI smoke that requires a live process**; runtime e2e; seal; `current-release.json`; GUI settings seal
- YouTube advisory remains advisory
- `windows.needs: [linux]` with `if: always() && !cancelled()` **only in L5, and L5 only after G1b**. L1–L4 leave `windows` parallel.
- `publish-qualification-evidence` on `push` to `main` is the **verifier**; windows is the **only** `gh release create`
- `artifact-gc` always for same-repo runs; never deletes git objects or Releases

Timeouts: linux 30 min, windows 60 min. The missing sanitizer target is small (`library-posix-native-filename-tests`); L1 must not bump timeout. If a future run hits 30 min, Grok packet is “split sanitizer into a second linux job,” not `-E` / `DISABLED` / drop tests.

### 4.3 Durable evidence (current contradiction)

Three documents disagree:

| Source | Claim |
|---|---|
| `EVIDENCE_POLICY.md` / `RELEASE_QUALIFICATION_REPORT.md` | Release assets `qualification-evidence-<commit>.zip` and `qualification-evidence-manifest-<commit>.json`; fail-closed identical rerun |
| Workflow `publish-qualification-evidence` | Writes compact `qualification-${GITHUB_SHA}.json` with linux/windows success flags and `gh release create` |
| `scripts/validate-qualification-publication.py` | Report: no `Qualification-Evidence/`, has `publish-qualification-evidence`, has `qualification-<full-source-commit>`. Workflow: job id, `contents: write`, compact JSON Python greps, `gh release create`, no `--clobber`, no `actions/upload-artifact@`, artifact-gc paths. Does **not** grep an evidence zip. D2 must keep those report substrings; put `C_main` in README / RELEASE_NOTES / TESTING_QUALIFICATION / SOURCE_COMMIT / issue #3. |

Gold state requires portable SHA-256 hashes bound to the qualified commit. The Windows job generates `SHA256SUMS.txt` and `current-release.json` in the runner workspace **after** GUI smoke. The publish job runs on a different runner and cannot see those files. Rebuilding the ZIP on Linux is impossible. Rebuilding on a second Windows job is not bit-identical.

**Rejected (Pass 1):** ephemeral `actions/upload-artifact@` plus rewriting the three `upload-artifact` forbids.

**Rejected (Pass 1):** `windows.needs: [linux]` **without** `if: always()`, which would skip Windows diagnosis when Linux fails.

**Rejected (Pass 2):** Windows job **polling** `gh run view $GITHUB_RUN_ID` until sibling linux completes. Failure modes:

- Job-level `permissions: contents: write` **replaces** (does not merge) workflow permissions, so `gh run view` can 403 without `actions: read`.
- 25-minute poll is shorter than linux `timeout-minutes: 30` → false fail.
- `cancel-in-progress: true` can leave a Release created by a cancelled run.
- Current ubuntu `publish-qualification-evidence` is a **second writer**: it `gh release create`s compact JSON only, then `exit 0` if the tag already exists **without** checking ZIP assets. Windows and ubuntu can disagree; compact JSON would become the false source of truth.

**Locked G2 design (L5 only after Grok restates it). Single writer. No poll. No upload-artifact.**

1. `windows` gains `needs: [linux]` and `if: ${{ always() && !cancelled() }}` so Windows still runs when Linux **fails** (Phase B diagnosis) and is skipped only when Linux is cancelled.
2. Wall-clock cost: linux duration + windows duration instead of `max()`. Acceptable. Do not drop tests to recover parallelism.
3. `windows` job-level permissions: `contents: write` **and** `actions: read`. The **only** `gh release create` in the workflow lives in a **last** windows step with:

   `if: github.event_name == 'push' && github.ref == 'refs/heads/main' && needs.linux.result == 'success' && success()`

    Top-level assets (only): compact `qualification-${GITHUB_SHA}.json`, portable ZIP, `current-release.json`. Inside the ZIP: `SHA256SUMS.txt`, `PORTABLE_MANIFEST.txt`, `RUNTIME_VERSIONS.txt`, `build-identity.json`, both exes, tools. Do not recompress after GUI-settings.
4. Compact JSON **must still be produced by the existing Python snippet** so `scripts/validate-qualification-publication.py` keeps matching `'commit': os.environ['GITHUB_SHA']`, `'run_id': os.environ['GITHUB_RUN_ID']`, and `qualification-${GITHUB_SHA}.json`. Do not switch that blob to PowerShell `ConvertTo-Json` (validator would fail; do not weaken the validator to match PowerShell).
5. No `--clobber`. If the tag exists, verify **prior** ZIP bytes equal **prior** `current-release.json.payload_sha256` and `commit == GITHUB_SHA`; peel the tag. Do **not** compare the newly built ZIP. Do **not** `exit 0` merely because the tag exists.
6. Ubuntu job id **remains** `publish-qualification-evidence` (validator requires that string). It `needs: [linux, windows]`, `if: push && main`, **creates nothing**. It only `gh release view`, asserts the ZIP asset exists, and checks `payload_sha256`. Source of truth: **the GitHub Release created by the windows job**. Ubuntu is a verifier. Two `gh release create` callers are forbidden.
7. `artifact-gc` unchanged. Policy tests keep `assert "actions/upload-artifact@" not in workflow`. Do not edit those three policy files in L5 unless adding **additional** positive asserts that do not remove forbids.
8. Docs: describe actual Release assets. Do not require a second evidence zip unless L5 builds it from the same bytes.

Until Gate G2, Luna must not add `actions/upload-artifact@` and must not edit the three policy forbids.

**L5 timing (Pass 4):** do **not** open L5 until G1b (both PR jobs green on the same head). L1–L4 must keep `windows` parallel with `linux` (no `needs: [linux]`). Do **not** open L9 until **G2b**: L5+L6 are on the new head **and** that head’s linux+windows jobs SUCCESS. `SHA_pr` for L10 **must contain L5+L6**. No merge-then-land-L5 shortcut.

### 4.3.1 Exact YAML Luna must insert in L5

Do not paraphrase. Edit `.github/workflows/archive-qt6.yml` as follows.

**A. Replace the `windows:` job header only** (leave existing steps until the new last step). Current header is `windows:` / `runs-on:` / `timeout-minutes:` / `steps:`. New header:

```yaml
  windows:
    needs: [linux]
    if: ${{ always() && !cancelled() }}
    runs-on: windows-2022
    timeout-minutes: 60
    permissions:
      contents: write
      actions: read
    steps:
```

Immediately after existing windows steps begin (after checkout + setup-python is enough), L5 must also add this **PR-safe** proof step. It must **not** call `gh release create`:

```yaml
      - name: Prove Git Bash can find Python
        shell: bash
        run: |
          set -euo pipefail
          PYBIN="$(command -v python.exe || command -v python3 || command -v python || true)"
          test -n "$PYBIN"
          "$PYBIN" -c "import sys; print(sys.executable)"
          command -v gh
          gh --version
```

After the identity step exists on PRs (it already runs when smoke passes), add this **PR-only dry-run** so G6 is not the first execution of the Python snippet. It must **not** call `gh release create` or `gh release upload`:

```yaml
      - name: Dry-run compact JSON and ZIP digest (pull_request only)
        if: github.event_name == 'pull_request'
        shell: bash
        run: |
          set -euo pipefail
          PYBIN="$(command -v python.exe || command -v python3 || command -v python || true)"
          test -n "$PYBIN"
          zip="portable/Media-Downloader-PS-Windows-Qt6-${GITHUB_SHA}.zip"
          identity="portable/current-release.json"
          test -f "$zip"
          test -f "$identity"
          "$PYBIN" - <<'PY'
          import hashlib, json, os
          from pathlib import Path
          sha = os.environ['GITHUB_SHA']
          ident = json.loads(Path('portable/current-release.json').read_text(encoding='utf-8'))
          digest = hashlib.sha256(Path('portable/Media-Downloader-PS-Windows-Qt6-' + sha + '.zip').read_bytes()).hexdigest()
          if ident.get('commit') != sha:
              raise SystemExit('dry-run commit mismatch')
          if ident.get('payload_sha256') != digest:
              raise SystemExit('dry-run payload_sha256 mismatch')
          path = Path('qualification-' + sha + '.json')
          path.write_text(json.dumps({
              'schema_version': 2,
              'repository': os.environ['GITHUB_REPOSITORY'],
              'commit': os.environ['GITHUB_SHA'],
              'run_id': os.environ['GITHUB_RUN_ID'],
              'run_attempt': os.environ['GITHUB_RUN_ATTEMPT'],
              'linux': 'success',
              'windows': 'success',
              'actions_artifacts': 'ephemeral; workflow GC removes them',
          }, indent=2, sort_keys=True) + '\n', encoding='utf-8')
          print('PR dry-run qualification publish: PASS')
          PY
```

This keeps the grepped Python keys in the workflow **twice** (dry-run + create). That is allowed. Still exactly one `gh release create`.

Top-level workflow `permissions: contents: read` stays. Job-level permissions replace, they do not merge: list **both** `contents: write` and `actions: read`. Fork PRs still get a read-only `GITHUB_TOKEN`. Same-repo PRs get write on this job.

**Pass 5/Terra 2 lock on write:** job-level `contents: write` plus ambient `GITHUB_TOKEN` authorize **every** windows step. The create step `if:` is a **step** guard, not the only authorization. Verbatim YAML must not call `gh release` / `gh api` writes except that guarded create step. L5 PR acceptance: windows log must not show `gh release create`; `gh release list --limit 20` must contain neither `qualification-<headRefOid>` nor `qualification-<job GITHUB_SHA>` (PR merge SHA). If either exists, STOP. L1–L4 must not apply this header or any `contents: write`. Do not prove create on PR.

**B. After step `GUI settings preserve package seal`, append this step.** Use `shell: bash` (Git Bash on `windows-2022`) so the validator grep strings stay byte-identical. Do **not** rewrite the Python dict in PowerShell.

```yaml
      - name: Publish compact immutable qualification identity
        if: github.event_name == 'push' && github.ref == 'refs/heads/main' && needs.linux.result == 'success' && success()
        shell: bash
        env:
          GH_TOKEN: ${{ github.token }}
        run: |
          set -euo pipefail
          tag="qualification-${GITHUB_SHA}"
          manifest="qualification-${GITHUB_SHA}.json"
          zip="portable/Media-Downloader-PS-Windows-Qt6-${GITHUB_SHA}.zip"
          identity="portable/current-release.json"
          PYBIN="$(command -v python.exe || command -v python3 || command -v python || true)"
          test -n "$PYBIN"
          "$PYBIN" - <<'PY'
          import json, os
          from pathlib import Path
          path = Path('qualification-' + os.environ['GITHUB_SHA'] + '.json')
          path.write_text(json.dumps({
              'schema_version': 2,
              'repository': os.environ['GITHUB_REPOSITORY'],
              'commit': os.environ['GITHUB_SHA'],
              'run_id': os.environ['GITHUB_RUN_ID'],
              'run_attempt': os.environ['GITHUB_RUN_ATTEMPT'],
              'linux': 'success',
              'windows': 'success',
              'actions_artifacts': 'ephemeral; workflow GC removes them',
          }, indent=2, sort_keys=True) + '\n', encoding='utf-8')
          PY
          test -f "$zip"
          test -f "$identity"
          test -f "$manifest"
          if gh release view "$tag" >/dev/null 2>&1; then
            mkdir -p /tmp/prior-qual
            gh release download "$tag" --pattern "qualification-${GITHUB_SHA}.json" --dir /tmp/prior-qual
            gh release download "$tag" --pattern "Media-Downloader-PS-Windows-Qt6-${GITHUB_SHA}.zip" --dir /tmp/prior-qual
            gh release download "$tag" --pattern "current-release.json" --dir /tmp/prior-qual
            "$PYBIN" - <<'PY'
          import hashlib, json, os, pathlib, sys
          sha = os.environ['GITHUB_SHA']
          prior = pathlib.Path('/tmp/prior-qual')
          ident = json.loads((prior / 'current-release.json').read_text(encoding='utf-8'))
          blob = (prior / ('Media-Downloader-PS-Windows-Qt6-' + sha + '.zip')).read_bytes()
          digest = hashlib.sha256(blob).hexdigest()
          if ident.get('commit') != sha:
              raise SystemExit('prior Release commit is not this GITHUB_SHA; do not overwrite')
          if ident.get('payload_sha256') != digest:
              raise SystemExit('prior ZIP bytes do not match prior current-release.json')
          print('Existing qualification Release is the immutable first publish; newly built ZIP is not compared (Compress-Archive is not byte-reproducible).')
          PY
            exit 0
          fi
          gh release create "$tag" "$manifest" "$zip" "$identity" --target "$GITHUB_SHA" \
            --title "Qualification $GITHUB_SHA" \
            --notes "Linux and Windows Archive Qt6 qualification executed and passed for this exact commit. Actions artifacts are ephemeral."
```

Required grep hits after this edit (must remain in the file):

- `qualification-${GITHUB_SHA}.json`
- `'commit': os.environ['GITHUB_SHA']`
- `'run_id': os.environ['GITHUB_RUN_ID']`
- `gh release create`
- `publish-qualification-evidence:`
- `contents: write`
- no `actions/upload-artifact@`
- no `--clobber`

**C. Replace the ubuntu job body** so it **verifies** and does not create. Keep the job id `publish-qualification-evidence:`. Remove its `gh release create`. Keep `contents: write` somewhere in the workflow (the windows job header satisfies the validator); ubuntu may use `contents: read`.

```yaml
  publish-qualification-evidence:
    name: publish durable qualification evidence
    if: github.event_name == 'push' && github.ref == 'refs/heads/main'
    needs: [linux, windows]
    runs-on: ubuntu-24.04
    timeout-minutes: 10
    permissions:
      contents: read
    steps:
      - name: Verify qualification release assets
        env:
          GH_TOKEN: ${{ github.token }}
        run: |
          set -euo pipefail
          tag="qualification-${GITHUB_SHA}"
          gh release view "$tag" --json tagName,assets > /tmp/rel.json
          python3 - <<'PY'
          import json, os, pathlib, urllib.request
          sha = os.environ['GITHUB_SHA']
          rel = json.loads(pathlib.Path('/tmp/rel.json').read_text())
          if rel.get('tagName') != 'qualification-' + sha:
              raise SystemExit('tag mismatch')
          names = {a['name'] for a in rel['assets']}
          need = {
              'qualification-' + sha + '.json',
              'Media-Downloader-PS-Windows-Qt6-' + sha + '.zip',
              'current-release.json',
          }
          missing = need - names
          if missing:
              raise SystemExit('missing release assets: ' + ','.join(sorted(missing)))
          PY
          python3 - <<'PY'
          import json, os, subprocess, sys
          sha = os.environ['GITHUB_SHA']
          ref = json.loads(subprocess.check_output(
              ['gh', 'api', f'repos/{os.environ["GITHUB_REPOSITORY"]}/git/ref/tags/qualification-{sha}'],
              text=True))
          obj = ref['object']
          if obj['type'] == 'commit':
              peeled = obj['sha']
          elif obj['type'] == 'tag':
              tagobj = json.loads(subprocess.check_output(
                  ['gh', 'api', obj['url']], text=True))
              peeled = tagobj['object']['sha']
          else:
              raise SystemExit('unexpected tag object type ' + obj['type'])
          if peeled != sha:
              raise SystemExit('tag does not peel to GITHUB_SHA: ' + peeled)
          print('tag peel: PASS')
          PY
          gh release download "$tag" --pattern "current-release.json"
          gh release download "$tag" --pattern "Media-Downloader-PS-Windows-Qt6-${GITHUB_SHA}.zip"
          python3 - <<'PY'
          import json, hashlib, os, pathlib
          sha = os.environ['GITHUB_SHA']
          ident = json.loads(pathlib.Path('current-release.json').read_text(encoding='utf-8'))
          blob = pathlib.Path('Media-Downloader-PS-Windows-Qt6-' + sha + '.zip').read_bytes()
          digest = hashlib.sha256(blob).hexdigest()
          if ident.get('commit') != sha:
              raise SystemExit('current-release.json commit mismatch')
          if ident.get('payload_sha256') != digest:
              raise SystemExit('payload_sha256 mismatch')
          if str(ident.get('run_id')) != str(os.environ['GITHUB_RUN_ID']):
              print('prior run_id retained (immutable first publish on this commit); this run did not replace the ZIP')
          print('qualification release verify: PASS')
          PY
```

`Select-String` on the workflow after L5 must find **exactly one** `gh release create`.

### 4.3.2 Payload ZIP layout (Compress-Archive)

Current identity step:

```powershell
Compress-Archive -Path (Join-Path $out '*') -DestinationPath $payloadZip
```

`$out` is `portable/Media-Downloader-PS`. The `*` glob zips **children**, not the folder. `Expand-Archive` therefore produces a **flat** tree: `media-downloader.exe` and `archive-cli.exe` at the extract root (not `extract/Media-Downloader-PS/...`).

L16b/L16c must:

1. Download the GitHub Release asset `Media-Downloader-PS-Windows-Qt6-<C_main>.zip` (the payload). Do not hash deno/ffmpeg inner archives, a local rebuild, or a re-zipped extract.
2. `payload_sha256` = SHA-256 of those ZIP **bytes**.
3. Extract to a fresh directory. Set `$pkg` to the directory that contains **both** `media-downloader.exe` and `archive-cli.exe` (`Get-ChildItem -Recurse` is allowed; if more than one `archive-cli.exe`, fail).
4. Do not `Compress-Archive` again after extract.

### 4.4 Target-host harness identity

`scripts/archive-local-harness.ps1` currently requires `-ExpectedArtifactId` (digits). It records that ID in evidence JSON. It does **not** fetch the artifact from GitHub. Actual trust is: external ZIP SHA-256, ZIP contents vs extracted tree, `build-identity.json` fields, and `SHA256SUMS.txt`.

After durable publication, the trusted ZIP is the GitHub Release asset, not an Actions artifact.

Packet L6 (after G2) must:

- accept `-ExpectedReleaseTag qualification-<commit>` and `-ExpectedArtifactSha256`
- keep ZIP digest binding
- stop requiring a perishable Actions artifact ID, **or** treat release asset ID as the identity field
- update `tests/archive-integration-tests.py` fixtures accordingly
- keep fail-closed seal checks

This is a contract clarification, not a weaken.

### 4.5 Extended qualification architecture

| Gate | Mechanism | Where |
|---|---|---|
| Endurance | `scripts/archive-endurance.ps1` ≥ 60 minutes, identity-bound to package commit **and** CI run | Target Windows host using the sealed Release ZIP. Gold 4 requires PASS. Missing host **pauses gold**; it is not UNAVAILABLE-as-PASS. |
| Low-memory | Defined procedure in Packet L16. Serial CTest `--parallel 1` of the Qt6 test build; record RAM. A Job-object/cgroup RSS cap is preferred but not a skip switch. | Windows local host (same as Luna) or Linux builder. Gold 4 requires PASS. |
| Portable layout | `SHA256SUMS.txt`, `RUNTIME_VERSIONS.txt`, pinned yt-dlp/deno/ffmpeg, `archive-cli.exe`, `media-downloader.exe`, translations, harness script | Windows CI after smoke + Release assets |
| Recovery | existing CTest archive-hardening / integration recovery cases in CI; target-host recovery package if media available | CI + optional host |
| Target-host media | `archive-local-harness.ps1` with authorized playlist/video URLs | External (credentials/media/hardware) |
| Advisory YouTube in CI | remains advisory; never the sole acceptance | Document unavailable if it fails |

### 4.6 Documentation architecture

Split docs into:

1. **Living method** (`docs/ARCHIVE_TESTING.md`, `EVIDENCE_POLICY.md`, `RELEASE_QUALIFICATION_REPORT.md` method sections) — no historical commit presented as current qualification.
2. **Current candidate identity** (`README.md` under qualification, `RELEASE_NOTES.md`, `TESTING_QUALIFICATION.md`, `SOURCE_COMMIT.txt`) — after Phase L only, bound to `C_main` and the successful main run. Until then they must say **unqualified candidate** or point at generated `current-release.json`.
3. **Historical** (`docs/qualification/history/`) — `4c720544` remains historical and unavailable as current evidence.
4. **Coordination** (`AUDIT2_INTEGRATION_STATUS.md`, issue #3) — current CI truth.

`scripts/validate-qualification-publication.py` must keep failing if current report cites `Qualification-Evidence/` local paths.

---

## 5. Exact merge sequence

### 5.1 Invariants

- Canonical working branch until merge: `audit2-remediation-all-251`
- Integration branch: `audit2-remediation`
- Release branch: `main`
- Never merge #213 while linux or windows is red, skipped, cancelled, or zero-step
- Never merge to `main` from the working branch directly
- Never rewrite `main`
- Required merge method: **merge commit**. Repo settings currently allow squash and rebase as well (`allow_squash_merge=true`, `allow_rebase_merge=true`). Luna must pass `gh pr merge --merge` and then prove two `parent` lines. If GitHub still squash-merges: **STOP. Do not `git revert` a 108-commit squash on a shared branch.** Return to Grok. Do not `--squash` / `--rebase` / `--admin`.

### 5.2 Sequence

```text
audit2-remediation-all-251 @ SHA_pr
        │
        │  Gate G2b+G3: SHA_pr contains L5+L6; linux+windows SUCCESS
        │  PR #213 still OPEN, MERGEABLE, checks green
        ▼
merge commit M_int on audit2-remediation
        │
        │  Gate G4: push workflow on audit2-remediation @ M_int
        │  linux+windows SUCCESS
        ▼
PR #N (successor-to-main): base main, head audit2-remediation
        │
        │  Gate G5: Archive Qt6 linux+windows SUCCESS on that PR head
        ▼
merge commit C_main on main
        │
        │  Gate G6: push workflow on main @ C_main
        │  linux+windows SUCCESS
        │  windows gh release create SUCCESS; publish-qualification-evidence verifier SUCCESS
        ▼
GitHub Release qualification-C_main
        │
        ▼
extended qualification + docs bind + ledger INTEGRATED
```

`SHA_pr` is the PR head that went green. `M_int` and `C_main` are **usually** different SHAs (merge commits). Do not assume they differ: after each merge run `git fetch origin; git rev-parse origin/audit2-remediation origin/main` and record the measured values in issue #3. If a fast-forward made `C_main == M_int`, still qualify that SHA with a **push** run on `main`, never by reusing the integration-branch run ID. Qualification of `C_main` is the release qualification. PR #213 run IDs are integration evidence only.

Concurrency: workflow group `archive-qt6-${{ github.ref }}` has `cancel-in-progress: true`. During Gate G6, do not push extra commits to `main` (including docs) until `R_main` finishes. A D2 push can cancel the qualifying run.

### 5.3 Successor-PR rules if HEAD moves

| Event | Rule |
|---|---|
| New commits on `audit2-remediation-all-251` while #213 open | Same PR. New head SHA. Re-run Archive Qt6. Merge only the green head. Update this plan's SHA table in issue #3, not by editing issue #2 |
| `audit2-remediation` moves (another merge) | Rebase or merge base into the working branch. If GitHub reports diverge, make #213 mergeable again. Do not merge with conflicts |
| #213 closed unmerged | Open successor PR with the same base `audit2-remediation` and a head that contains the 251 combined tree. Record successor number in issue #3. All #213 merge gates apply to the successor |
| Accidental commits on a new branch | Do not abandon `audit2-remediation-all-251` without Grok. One combined head only |
| Need for CI-only fix after this plan | Commit on `audit2-remediation-all-251`. Do not open a parallel "ci-fix" PR against `main` |
| Finding `MDPS-AUDIT2-252+` confirmed in issue #2 before `C_main` | Stop Phase E+. Grok decides whether to extend the combined branch or cut a follow-on integration. Do not silently ship 251 as complete if 252 is critical/high |

### 5.4 Merge-day commands (Luna, only inside the matching packet)

Verify, do not assume:

```text
gh pr view 213 --json state,mergeable,mergeStateStatus,headRefOid,baseRefOid,statusCheckRollup
gh run list --branch audit2-remediation-all-251 --workflow "Archive Qt6 qualification" --limit 5
```

Both `linux` and `windows` check runs for `headRefOid` must be `SUCCESS` with non-empty executed steps.

Then, only when the packet says so:

```text
gh pr merge 213 --merge --delete-branch=false
```

Do not delete the working branch.

After merge:

```text
git fetch origin
git rev-parse origin/audit2-remediation
gh run list --branch audit2-remediation --workflow "Archive Qt6 qualification" --limit 5
```

Wait for that run. Inspect job steps. Zero-step failure is not green.

---

## 6. Qualification matrix

Every cell must be `PASS` or `UNAVAILABLE (documented)`. Never blank. Never inferred.

| ID | Gate | Environment | Current | Required evidence | Packet |
|---|---|---|---|---|---|
| Q1 | Python policy compile | GHA linux + windows | PASS on `bebe42d` | `compileall -q tests` success | B |
| Q2 | Publication validator | GHA linux | PASS | `scripts/validate-qualification-publication.py` | B / C |
| Q3 | Linux Qt6 Debug build | GHA linux | PASS | cmake + ninja | B |
| Q4 | Linux unsanitized CTest | GHA linux | PASS | full suite including `library-posix-native-filename` | B |
| Q5 | Linux sanitizer configure | GHA linux | PASS | `BUILD_TESTING=ON`, ASan+UBSan flags | B |
| Q6 | Linux sanitizer build | GHA linux | PASS build of filtered targets; **missing native-filename exe** | full sanitizer tree + `test -x` | L1 |
| Q7 | Linux sanitizer CTest | GHA linux | FAIL Not Run 203 | all tests run; 203 Passed; no ASan/UBSan halt | L1 / L4 |
| Q8 | Windows Qt6 Release build | GHA windows | PASS | cmake ninja | B |
| Q9 | Windows CTest | GHA windows | PASS | full suite | B |
| Q10 | Windows symlink probe | GHA windows | PASS | workflow probe | B |
| Q11 | Portable assemble + tool SHA-256 | GHA windows | PASS | pinned yt-dlp/deno/ffmpeg hashes | B |
| Q12 | Packaged preflight | GHA windows | PASS | `archive-cli.exe preflight` | B |
| Q13 | Portable GUI smoke | GHA windows | FAIL exit 1 | process alive 5s, default launch | L2 / L3 |
| Q14 | Packaged runtime e2e | GHA windows | SKIPPED | yt-dlp+ffmpeg+ffprobe local media | after L3 |
| Q15 | GUI settings package seal | GHA windows | SKIPPED | `tests/archive-gui-package-immutability.ps1` | after L3 |
| Q16 | `SHA256SUMS.txt` + `current-release.json` generated | GHA windows | SKIPPED | identity bound to `GITHUB_SHA` and `GITHUB_RUN_ID` | after L3 |
| Q17 | YouTube advisory CI | GHA windows | not yet run on this head | advisory only; never sole PASS | after L3 |
| Q18 | Artifact GC | GHA | PASS | artifacts/caches deleted; refs preserved | all runs |
| Q19 | PR #213 both jobs green | GHA | FAIL | run on exact merge head | G3 |
| Q20 | `audit2-remediation` both jobs green | GHA | not yet | run on `M_int` | G4 |
| Q21 | main-candidate PR both jobs green | GHA | not yet | run on that PR | G5 |
| Q22 | `main` push both jobs green | GHA | not yet | run on `C_main` | G6 |
| Q23 | Durable Release `qualification-C_main` | GitHub Release | not yet | tag, compact JSON, portable ZIP, SHA-256 | L5 / Phase J |
| Q24 | Recovery tests | GHA CTest + optional host | linux/windows CTest include recovery cases when jobs complete | CTest names containing recovery/import/tamper | B + K |
| Q25 | Endurance ≥ 60 min | target Windows host | not run for this candidate | `archive-endurance.json` bound to `C_main` and main run ID. PASS required for gold 4. Missing host pauses gold. | Phase K |
| Q26 | Low-memory | Windows local host | procedure defined in L16; not yet run | serial CTest `--parallel 1` PASS log + RAM note. Not skippable as UNAVAILABLE. | Phase K |
| Q27 | Target-host media | target Windows host | blocked on durable ZIP + authorized URLs | `local-harness-evidence-*.json`. Gold says "where applicable"; missing media is documented and pauses that bullet, not endurance/low-memory. | Phase K |
| Q28 | Docs bind | git on `main` | stale `4c720544` / runner-allocation | living docs cite `C_main` | Phase L |
| Q29 | Issue #3 records `C_main` + run ID | GitHub issue | stale | append-only UPDATE | Phase L |
| Q30 | No new secrets | git / GitHub | must remain true | `gh secret` not committed; no tokens in logs | all |

CI YouTube advisory may be documented unavailable (gold 4 last bullet: advisory tests). Q25 endurance requires operator URLs the endurance script can scan/sync (not hostname-specific). Missing those URLs **pauses gold 4**; it is not UNAVAILABLE-PASS and not Q27. Q26 missing Qt pauses gold 4. Q27 is the live-media harness “where applicable.”

---

## 7. Evidence and identity binding

### 7.1 Required identity chain for the published release

```text
origin/main == C_main
→ Archive Qt6 push run on C_main (event=push, ref=refs/heads/main, headSha=C_main)
→ linux job success with real steps
→ windows job success with real steps
→ portable ZIP Z = Media-Downloader-PS-Windows-Qt6-C_main.zip
→ GitHub Release tag qualification-C_main, peeled commit C_main
→ top-level Release assets only: compact JSON, Z, current-release.json
→ **R_main := current-release.json.run_id on that Release** (first publish). Not `gh run list --limit 1`. A G6 retry that keeps the prior Release still PASSes; R_main stays the sealed run_id.
→ SHA256SUMS.txt inside the portable tree (not a top-level Release asset)
→ docs and issue #3 cite C_main and R_main
→ endurance and local harness ExpectedCommit=C_main, ExpectedCiRun=R_main (sealed run_id)
```

No field may be copied from PR run `35586304364` or historical `35116148963`.

### 7.1.1 `C_main` vs docs-follow-up identity

`C_main` is the qualified **product** commit. Packet L17 may add a later docs-only commit `C_docs` on `main`.

**H5 lock (option 3):** D2 push to `main` **will** run Archive Qt6 and **will** create sibling Release `qualification-C_docs` if linux+windows succeed. Do not add a skip-guard in L5. Do not move tag `qualification-C_main`. Gold identity is **only** `qualification-C_main`. `qualification-C_docs` is a non-gold sibling. G9 FAIL if README / RELEASE_NOTES / SOURCE_COMMIT name `C_docs` as the qualified product. L15/G7 hash **only** the `qualification-C_main` ZIP.

| Object | Must name |
|---|---|
| Gold Git tag / Release | `qualification-C_main` only. Never move to `C_docs`. |
| `current-release.json` used for gold | the one on `qualification-C_main` |
| Endurance / harness `ExpectedCommit` | `C_main` |
| Living docs after D2 | "`C_main` is the qualified product commit. `C_docs` is documentation of that commit and is not requalified." |
| `origin/main` after D2 | May be `C_docs`. Not a false-done for product identity. |

If D2 accidentally changes workflow or sources, it is not docs-only: stop and requalify.

### 7.2 File-level contracts

**`current-release.json`** (generated by Windows job, already specified in workflow):

- `schema_version`
- `repository`
- `git_ref`
- `commit` == `GITHUB_SHA`
- `run_id` == `GITHUB_RUN_ID`
- `run_attempt` == `GITHUB_RUN_ATTEMPT`
- `artifact_name`
- `payload_file`
- `payload_sha256`

**`SHA256SUMS.txt`**: SHA-256 of every packaged file except itself, lowercase hex, `  ` separator, `/` paths.

**`build-identity.json`**: after seal, `qualification=windows-ci-qualified-for-local-harness` and matching commit/run.

**GitHub Release compact JSON**: repository, commit, run_id, run_attempt, linux=success, windows=success.

**Git tag**: `qualification-<full 40-char commit>`. Create with `--target "$GITHUB_SHA"`. Never move the tag. Prove identity by peeling the git ref to the commit SHA (do not assert API `targetCommitish == sha`; that field is often `main`).

### 7.3 Fail-closed republish

If `qualification-C_main` already exists, do not `--clobber`. First successful create is the payload. `Compress-Archive` is not byte-reproducible; do **not** `cmp` a newly built ZIP against the Release ZIP.

On retry of the same `C_main`: download prior ZIP + `current-release.json`; require `commit == C_main` and `sha256(prior ZIP) == payload_sha256`; peel tag to `C_main`; keep prior assets; exit 0. If prior `commit` is some other SHA, fail. Do not treat a new local ZIP digest as a new qualification.

### 7.4 What is not identity

- PR artifacts (none should exist; GC deletes them)
- `docs/qualification/history/*`
- chat
- Luna summaries
- hashes computed on a locally rebuilt tree that is not ZIP Z

---

## 8. Documentation updates required

Two waves. Do not collapse them.

### Wave D1 — Stop lying about current CI (before merge, on working branch)

Files that currently over-claim runner-allocation or historical qualification as current:

| File | Problem | Required D1 change |
|---|---|---|
| `docs/qualification/AUDIT2_INTEGRATION_STATUS.md` | Method is mostly correct; does not record run `35586304364` product failures | Add "Current executable CI" subsection: linux sanitizer Not Run 203; windows GUI exit 1; run ID; "not a runner-allocation blocker" |
| `docs/remediation/FINAL-AUDIT2-RECONCILIATION.md` | Entire CI column is `BLOCKED: exact-head runners failed before steps`; qualification section cites `35502470214` | Add a dated addendum at the top: superseded CI reality as of `bebe42d` / run `35586304364`. Do not rewrite the historical matrix rows (they are a snapshot). |
| `docs/qualification/README.md` | States candidate is `4c720544` / `35116148963` | Replace with: this directory describes method and historical records; current candidate is unqualified until `main` `qualification-<commit>` exists |
| `docs/qualification/TESTING_QUALIFICATION.md` | Same historical claim presented as final candidate | Same: method only; historical numbers belong in `history/` |
| `docs/qualification/RELEASE_NOTES.md` | Same | Header: not the current qualified release notes |
| `AUDIT-FIX-LIST.md` | Cites `35502470214` empty steps | Addendum, do not fabricate a new QUALIFIED row |
| Issue #3 | Last comment empty-step | Append UPDATE (see §9). Not a repo file |
| `docs/qualification/GOLD-STATE-EXECUTION-PLAN.md` | Untracked planning artifact | Commit it in L7 only. Keep the header: not qualification evidence, not a release claim. Do not point `SOURCE_COMMIT.txt` at it. |

Do not put `C_main` into these files during D1. It does not exist.

### Wave D2 — Bind to `C_main` (after Gate G6 + Phase J + K)

Only after GitHub Release exists:

- `docs/qualification/README.md`
- `docs/qualification/RELEASE_NOTES.md`
- `docs/qualification/TESTING_QUALIFICATION.md`
- `docs/qualification/SOURCE_COMMIT.txt` (may remain "generated by workflow" plus pointer to the Release)
- `docs/qualification/AUDIT2_INTEGRATION_STATUS.md`
- `docs/qualification/RELEASE_QUALIFICATION_REPORT.md` current-candidate section
- `docs/qualification/EVIDENCE_POLICY.md` aligned with actual Release assets
- `changelog` if the project records user-facing release notes there

Each current-candidate document must include:

- commit `C_main`
- run `R_main`
- release tag `qualification-C_main`
- payload SHA-256 from `current-release.json`
- what was fixed (audit2 through 251 plus CI launch/sanitizer repairs)
- what was tested (matrix §6)
- advisory/unavailable checks named explicitly

`scripts/validate-qualification-publication.py` must still pass. If D2 text would fail the validator, change the text or extend the validator **with Grok review**, not by removing checks.

---

## 9. Issue #2 / #3 ledger protocol

### 9.1 Issue #2

- Immutable discovery ledger.
- Do not edit or delete finding comments.
- Do not post FIXED there as a rewrite of discovery status.
- New findings get new IDs after fetching the current highest number.
- If a comment is needed, it is a **new** comment referencing an ID, never an edit.

### 9.2 Issue #3

Append-only. Use the existing templates.

Immediately after plan freeze (Grok or Luna Packet L8, no code required):

```text
UPDATE CI-TRUTH
Status: BLOCKED
Branch: audit2-remediation-all-251
PR: #213
Commit(s): bebe42d480db763441f5a22a7c3c7c456434a87d
CI: run 35586304364 FAILURE (executable product steps)
Linux job 106290346908: sanitizer CTest 203/204 library-posix-native-filename Not Run; missing build-sanitized/library-posix-native-filename-tests. Unsanitized Test step succeeded. Root cause: sanitizer cmake --build --target list omits that executable.
Windows job 106290346708: Portable GUI launch smoke exit 1. CTest, portable assemble, preflight succeeded. Subsequent seal/hash/runtime steps skipped.
This supersedes comments that described runner-allocation / steps: [] for later heads.
Residual risk: GUI root cause not yet evidenced; sanitizer test 203 not yet executed under ASan.
No FIXED/INTEGRATED claim.
```

After each packet that pushes:

```text
UPDATE MDPS-AUDIT2-251 / CI
Status: PR-OPEN | ...
Commit(s): <sha>
CI: <run> <conclusion>
Regression evidence: <what ran>
Residual risk: ...
```

INTEGRATED is allowed only when the fix is on `audit2-remediation` **and** Gate G4 is green.

FIXED for release-line purposes is recorded only after `C_main` exists. Prefer: "INTEGRATED on audit2-remediation @ M_int" then "QUALIFIED on main @ C_main / run R_main / release qualification-C_main".

### 9.3 Claims

Do not CLAIM new finding IDs for the sanitizer target-list bug or GUI smoke unless Grok opens a new audit ID. They are qualification blockers on already-remediated work, tracked as CI in issue #3.

---

## 10. Luna execution packets

Each packet is independently executable. Luna may not start packet N+1 without the matching Grok gate. **L1 and L2 are not opened together.** G0, when later opened, opens L1 (optionally L7 and L8). L2 opens after G1-linux. L7/L8 must not edit `.github/workflows/archive-qt6.yml`.

Global Luna setup for every packet:

```text
git fetch origin
git checkout audit2-remediation-all-251
git pull --ff-only origin audit2-remediation-all-251
git status
git rev-parse HEAD
```

HEAD must match the packet's expected SHA unless the packet says "commit on current HEAD". Do not commit `tests/__pycache__/`. Do not `git add -A`.

Report format for every packet:

```text
PACKET: L#
HEAD_BEFORE: <sha>
HEAD_AFTER: <sha or unchanged>
FILES: <paths>
COMMANDS_RUN: ...
GITHUB_RUN: <id or none>
RESULT: PASS | FAIL | BLOCKED
EVIDENCE: <urls, log excerpts, hashes>
BLOCKER: <none or description>
NEXT: wait for Grok
```

### Packet L1 — Sanitizer target completeness

**Goal:** Sanitizer CTest can execute `library-posix-native-filename`.

**Files:** `.github/workflows/archive-qt6.yml` only, unless a proof script is required.

**Change:**

1. Replace sanitizer `--target ...` build with `cmake --build build-sanitized --parallel 2`.
2. Add step immediately after build:

```yaml
- name: Prove sanitizer native-filename binary exists
  run: test -x build-sanitized/library-posix-native-filename-tests
```

**Expected local diff intent:** workflow YAML only, linux job sanitizer steps only. Do not edit the windows job in this packet.

**Luna commands (PowerShell):**

```powershell
git fetch origin
git checkout audit2-remediation-all-251
git pull --ff-only origin audit2-remediation-all-251
git diff -- .github/workflows/archive-qt6.yml
# after edit:
git add -- .github/workflows/archive-qt6.yml
git commit -m "fix(ci): build sanitizer native-filename tests"
git status
git push origin audit2-remediation-all-251
gh pr view 213 --json headRefOid
gh run list --branch audit2-remediation-all-251 --workflow "Archive Qt6 qualification" --limit 3
```

Do **not** run `test -x` or sanitizer CMake on this Windows host as L1 acceptance.

**Acceptance:**

- New SHA ≠ `bebe42d480db763441f5a22a7c3c7c456434a87d` unless GitHub already moved (then use the live head)
- Linux job on that SHA: proof step success if added; sanitizer CTest does not list `library-posix-native-filename (Not Run)`
- L1 PASS if linux sanitizer step succeeds **or** test 203 ran and failed with a real assertion/ASan (FAIL-PRODUCT → Grok opens L4)
- Windows red does **not** fail L1. Windows red also does **not** complete Phase B.

**Forbidden:** modifying `tests/library-posix-native-filename-tests.cpp`, CMake `if(UNIX)` removal, CTest exclude, policy tests, windows smoke step, combining with L2.

**Rollback:** `git revert` of the CI commit; push.

### Packet L2 — Windows GUI smoke diagnosis (no pass-condition change)

**Goal:** Next windows failure, if any, includes plugin/DLL/process/log evidence. Default smoke still throws on exit.

**Files:** `.github/workflows/archive-qt6.yml` Portable GUI launch smoke step.

**Change sketch (keep throw). This YAML runs on the windows-2022 runner, not on Luna's laptop as acceptance:**

```powershell
$ErrorActionPreference = 'Stop'
$out = Join-Path $env:GITHUB_WORKSPACE 'portable/Media-Downloader-PS'
$logDir = Join-Path $env:RUNNER_TEMP 'portable-gui-smoke'
New-Item -ItemType Directory -Force $logDir | Out-Null
Get-ChildItem $out -Recurse | Select-Object FullName,Length | Format-Table -AutoSize | Out-String -Width 4096 | Set-Content (Join-Path $logDir 'tree.txt')
Write-Host "tree written $(Join-Path $logDir 'tree.txt')"
$required = @(
  'media-downloader.exe',
  'platforms/qwindows.dll',
  'Qt6Core.dll',
  'Qt6Gui.dll',
  'Qt6Widgets.dll',
  'libstdc++-6.dll',
  'libgcc_s_seh-1.dll',
  'libwinpthread-1.dll'
)
foreach ($rel in $required) {
  $p = Join-Path $out $rel
  if (!(Test-Path -LiteralPath $p)) { Write-Host "MISSING $rel" } else { Write-Host "PRESENT $rel size=$((Get-Item -LiteralPath $p).Length)" }
}
Write-Host 'PRE-EXISTING PROCESSES'
Get-Process | Where-Object { $_.Name -match 'media-downloader' } | Format-Table Id,Name,StartTime -AutoSize | Out-String | Write-Host
Write-Host 'SESSION (quser is noise; winlogon is the pause predicate)'
quser 2>&1 | Write-Host
Get-Process winlogon -ErrorAction SilentlyContinue | Format-Table Id,ProcessName | Out-String | Write-Host
$env:QT_DEBUG_PLUGINS = '1'
$env:QT_LOGGING_RULES = 'qt.qpa.*=true'
$env:QT_ASSUME_STDERR_HAS_CONSOLE = '1'
$process = Start-Process -FilePath (Join-Path $out 'media-downloader.exe') -WorkingDirectory $out -PassThru
Start-Sleep -Seconds 5
$process.Refresh()
Write-Host "HasExited=$($process.HasExited) ExitCode=$(if ($process.HasExited) { $process.ExitCode } else { 'running' })"
$dumps = Join-Path $env:LOCALAPPDATA 'CrashDumps'
if (Test-Path -LiteralPath $dumps) { Get-ChildItem -LiteralPath $dumps | ForEach-Object { Write-Host $_.FullName } }
if ($process.HasExited) { throw "Portable GUI exited during smoke test with code $($process.ExitCode)" }
Stop-Process -Id $process.Id -Force
```

Do not add `-RedirectStandardOutput` or `-RedirectStandardError` in L2. Do not add `-ArgumentList '-s'`. Put tree/DLL logs before the throw.

**Commit message:**

```text
fix(ci): capture portable GUI smoke diagnostics
```

**Luna commands:** same git fetch/ff-only as L1, then edit only the Portable GUI launch smoke step, `git add -- .github/workflows/archive-qt6.yml`, commit, push. Confirm the sanitizer L1 hunk is still present (`Select-String -LiteralPath .github/workflows/archive-qt6.yml -Pattern 'library-posix-native-filename-tests'`).

**Acceptance:** YAML still contains `throw "Portable GUI exited during smoke test with code`. Next windows job log contains `PRESENT`/`MISSING` lines. Liveness assertion still throws on exit 1. Empty stderr is expected for `WIN32` and is not a packet failure.

**Forbidden:** `continue-on-error: true`; `QT_QPA_PLATFORM=offscreen`; reducing sleep below 5; `exit 0` after failure; combining with L1; Xvfb; changing `WIN32` to console just to capture logs.

**Rollback:** `git revert` of the L2 commit only.

### Packet L3 — Windows GUI root-cause fix

**Precondition:** G1-windows opened L3 because smoke **still throws**. If smoke SUCCESS, L3 is SKIP — do not rewrite a green smoke. Grok listed exact files. Luna does not guess files.

**Files:** only those listed in Grok's L3 note. Expected candidates:

- portable assemble block in `.github/workflows/archive-qt6.yml` (DLL/plugin copy)
- `src/main.cpp` / `src/utils/single_instance.hpp` / settings / theme (product)

Do **not** edit “GUI session setup.” No offscreen, no Xvfb, no `continue-on-error`. If DLLs are present and there is no interactive session, return to Grok; gold 3 pauses.

**Acceptance:** windows job Portable GUI launch smoke SUCCESS; runtime e2e, seal, `current-release.json` steps execute; assertion not weakened.

**Forbidden:** any change to policy tests to ignore GUI; marking smoke advisory.

If L2 evidence is insufficient, Luna retries L2 once with additional logging specified by Grok. Second insufficient capture returns to Grok; no speculative product rewrite.

### Packet L4 — Sanitizer test 203 real failure (conditional)

Open only if L1 caused test 203 to **run** and fail.

Fix `tests/library-posix-native-filename-tests.cpp` and/or `src/directoryEntries.cpp` so ASan/UBSan pass. Keep the invalid-byte and collision invariants.

**Forbidden:** skipping sanitizer; leak suppression files unless Grok approves a documented suppression with remaining-risk text.

### Packet L5 — Durable Release single-writer (after Gate G2)

Implement §4.3.1 **verbatim YAML**. Do not paraphrase the Python snippet. Default file list:

- `.github/workflows/archive-qt6.yml` (required)
- `docs/qualification/EVIDENCE_POLICY.md`
- `docs/qualification/RELEASE_QUALIFICATION_REPORT.md`

Do **not** open `tests/ci-supply-chain-policy-tests.py`, `tests/audit2-final-integration-policy-tests.py`, or `scripts/validate-qualification-publication.py` unless Grok's G2 note lists a **new positive assert**. Removing or narrowing `actions/upload-artifact@` forbids is forbidden. Keep the Python compact-JSON snippet strings the validator greps for. Keep job id `publish-qualification-evidence:`. L5 docs may describe H6 top-level assets but must keep report substrings `qualification-<full-source-commit>`, `publish-qualification-evidence`, and policy `ordinary Actions artifact expiry`.

Policy tests must still:

- pin actions by SHA
- pin Debian digest
- forbid `--clobber`
- require GC
- forbid Qt `cache: true`
- compileall both OS

Do **not** replace the `actions/upload-artifact@` forbids. After L5 those three files must still contain `assert "actions/upload-artifact@" not in workflow` or `require("actions/upload-artifact@" not in workflow`.

**Luna local policy check (PowerShell, after edit, not a substitute for GitHub):**

```powershell
python tests/ci-supply-chain-policy-tests.py --source-root .
python tests/audit2-final-integration-policy-tests.py --source-root .
python scripts/validate-qualification-publication.py --source-root .
Select-String -LiteralPath .github/workflows/archive-qt6.yml,tests/ci-supply-chain-policy-tests.py,tests/audit2-final-integration-policy-tests.py,scripts/validate-qualification-publication.py -Pattern 'upload-artifact'
```

**Acceptance:** those commands print PASS; workflow has no `actions/upload-artifact@`; exactly one `gh release create`; PR windows log contains `PR dry-run qualification publish: PASS` once smoke+identity have run; `gh release list` has neither `qualification-<headRefOid>` nor `qualification-<GITHUB_SHA>`; ubuntu job has no `gh release create`; GitHub linux Q2 still passes. Do not claim gold until a `main` run creates `qualification-C_main` with the ZIP.

### Packet L6 — Local harness binds to durable ZIP

**Files:** `scripts/archive-local-harness.ps1`, `tests/archive-integration-tests.py`, any policy that snapshots harness flags.

Keep SHA-256 ZIP binding. In `scripts/archive-local-harness.ps1` param block, **replace** the `ExpectedArtifactId` parameter line with:

```powershell
[Parameter(Mandatory=$true)][ValidatePattern('^qualification-[0-9a-f]{40}$')][string]$ExpectedReleaseTag,
```

Record `release_tag` in evidence JSON **instead of** `artifact_id=$ExpectedArtifactId`. After the edit, `Select-String` on the ps1 must find **zero** `ExpectedArtifactId`. In `tests/archive-integration-tests.py`, where `commit` is already `'a' * 40`, pass `-ExpectedReleaseTag ('qualification-' + commit)` and assert `evidence['release_tag']`. Keep ZIP byte digest and seal mismatch throws. Production ZIP is **flat** (§4.3.2); the integration fixture may stay nested — do not change `Compress-Archive` glob in L1–L4. L6 PASS is GitHub `archive-integration` on the L6 head, not `--help`.

**Acceptance (PowerShell):**

```powershell
Select-String -LiteralPath scripts/archive-local-harness.ps1,tests/archive-integration-tests.py -Pattern 'ExpectedArtifactId'
```

Zero matches. GitHub CTest `archive-integration` on the L6 head is the real gate.

### Packet L7 — Docs wave D1

**Files:** list in §8 Wave D1, including `docs/qualification/GOLD-STATE-EXECUTION-PLAN.md`, `docs/qualification/TERRA-HIGH-1-CRITIQUE.md`, and `docs/qualification/TERRA-HIGH-2-CRITIQUE.md` (planning artifacts, not `C_main` identity).

**Acceptance:** no current-candidate document claims `4c720544` is the live qualified commit; no live doc claims current CI is empty-step runner allocation; historical files untouched except references that already exist; validator still PASS; `git ls-files docs/qualification/GOLD-STATE-EXECUTION-PLAN.md` is non-empty.

### Packet L8 — Issue #3 CI-truth comment

No product code. Write the §9.2 UPDATE to a UTF-8 file without BOM, then:

```powershell
$body = @'
UPDATE CI-TRUTH
Status: BLOCKED
Branch: audit2-remediation-all-251
PR: #213
Commit(s): bebe42d480db763441f5a22a7c3c7c456434a87d
CI: run 35586304364 FAILURE (executable product steps)
Linux job 106290346908: sanitizer CTest 203/204 library-posix-native-filename Not Run.
Windows job 106290346708: Portable GUI launch smoke exit 1.
No FIXED/INTEGRATED claim.
'@
[IO.File]::WriteAllText("$env:TEMP\issue3-ci-truth.md", $body)
gh issue comment 3 --body-file $env:TEMP\issue3-ci-truth.md
```

**Forbidden:** editing old comments; commenting on issue #2 except if Grok orders a new ID.

### Packet L9 — Wait / verify PR #213 green

No code. Commands (PowerShell):

```powershell
gh pr view 213 --json headRefOid,statusCheckRollup,mergeable,mergeStateStatus
$oid = (gh pr view 213 --json headRefOid --jq .headRefOid)
$run = gh run list --branch audit2-remediation-all-251 --workflow "Archive Qt6 qualification" --limit 5 --json databaseId,headSha,conclusion,event | ConvertFrom-Json | Where-Object { $_.headSha -eq $oid -and $_.event -eq 'pull_request' } | Select-Object -First 1
if (-not $run) { throw 'no PR run for current head' }
gh run view $run.databaseId --json conclusion,jobs
```

PASS only if linux and windows `conclusion=success` and each has executed product steps (configure/build/test/smoke as applicable). Publish skipped is OK on pull_request.

### Packet L10 — Merge #213 into `audit2-remediation`

**Precondition:** Gate G3.

```powershell
gh api repos/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified --jq "{allow_merge_commit,allow_squash_merge,allow_rebase_merge}"
gh pr view 213 --json state,mergeable,mergeStateStatus,headRefOid,statusCheckRollup
gh pr merge 213 --merge --delete-branch=false
if ($LASTEXITCODE -ne 0) { throw 'gh pr merge --merge failed; STOP' }
git fetch origin
$M_int = (git rev-parse origin/audit2-remediation).Trim()
$parents = @(git rev-list --parents -n 1 $M_int).Split(' ')
# rev-list --parents prints: <commit> <parent1> [parent2 ...]
if ($parents.Count -lt 3) {
  throw "origin/audit2-remediation $M_int is not a merge commit (parent count=$($parents.Count-1)). Do not revert. STOP for Grok."
}
Write-Host "M_int=$M_int parent1=$($parents[1]) parent2=$($parents[2])"
```

Record `M_int`. Do not merge to main. Do not `--squash` / `--rebase` / `--admin`. Do not `git revert` / `git reset` `audit2-remediation` if this proof fails.

### Packet L11 — Verify `audit2-remediation` workflow

Wait for push run on `audit2-remediation`. Same success rule as L9. If GitHub skips because of concurrency, wait for the surviving run.

### Packet L12 — Open PR `audit2-remediation` → `main`

```powershell
gh pr create --base main --head audit2-remediation --title "Integrate audit2-remediation through MDPS-AUDIT2-251" --body @"
Integrates verified ``audit2-remediation`` @ ``<M_int>`` (merge of #213).
Not a gold-state claim. Qualification is the subsequent push run on main.
"@
```

Replace `<M_int>` with the 40-character SHA from L11. Body must say gold is not claimed.

### Packet L13 — Merge to `main`

**Precondition:** Gate G5. Let `N` be the PR number from L12.

```powershell
gh pr view $N --json state,mergeable,mergeStateStatus,headRefOid,statusCheckRollup
gh pr merge $N --merge --delete-branch=false
if ($LASTEXITCODE -ne 0) { throw 'gh pr merge --merge failed; STOP' }
git fetch origin
$C_main = (git rev-parse origin/main).Trim()
$parents = @(git rev-list --parents -n 1 $C_main).Split(' ')
if ($parents.Count -eq 2) {
  Write-Host "C_main=$C_main is a single-parent commit (fast-forward). Qualify this SHA with a main push run."
} elseif ($parents.Count -ge 3) {
  Write-Host "C_main=$C_main parent1=$($parents[1]) parent2=$($parents[2])"
} else {
  throw "Unexpected parent list for $C_main"
}
# Squash detection: single parent AND commit is not equal to origin/audit2-remediation
$Mint = (git rev-parse origin/audit2-remediation).Trim()
if ($parents.Count -eq 2 -and $C_main -ne $Mint) {
  $msg = git log -1 --format=%s $C_main
  if ($msg -notmatch '^Merge ') {
    throw "Likely squash onto main: $C_main message='$msg'. Do not revert. STOP for Grok."
  }
}
```

Never `--admin`. Do not `git reset` `main`.

### Packet L14 — Qualify `C_main`

Wait for a push run on `C_main` with linux+windows SUCCESS and Release `qualification-C_main` present (ZIP + identity). G6 PASS even if this run kept a prior Release. Then set `$Token.R_main` from **downloaded** `current-release.json.run_id`, not from `gh run list --limit 1`. False-done: latest green main run ID when it differs from the Release identity. Do not drop `ExpectedCiRun`.

### Packet L15 — Confirm Release assets

Do not run with `<>` in the tag. Bind `$Cmain` to the recorded `C_main` token, never to `origin/main` after D2.

```powershell
$Cmain = $Token.C_main
if ($Cmain -notmatch '^[0-9a-f]{40}$') { throw 'C_main token missing' }
gh release view "qualification-$Cmain"
gh api "repos/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified/git/ref/tags/qualification-$Cmain"
# peel: object.sha == C_main (or annotated tag object's commit sha)
gh release download "qualification-$Cmain" --pattern "Media-Downloader-PS-Windows-Qt6-$Cmain.zip"
gh release download "qualification-$Cmain" --pattern "current-release.json"
# Get-FileHash the ZIP bytes; must equal current-release.json.payload_sha256
# Expand and confirm SHA256SUMS.txt is inside the tree, not a fourth Release asset
```

### Packet L16 — Extended qualification

Split this packet at execution time into L16a low-memory, L16b endurance, L16c harness. Grok opens each when the prior required SHA/ZIP exists. Gold 4 does not advance until L16a and L16b PASS. L16c missing media pauses Q27 only.

Named BLOCKED states (not PASS, not silent skip):

| State | Trigger | Gold |
|---|---|---|
| `GOLD-4-PAUSED-LOWMEM` | L16a missing cmake/ninja/Qt or HEAD ≠ `C_main` | Gold 4 pauses. Issue #3 UPDATE. L1–L15 may continue. |
| `GOLD-4-PAUSED-ENDURANCE` | L16b missing host or operator URLs | Gold 4 pauses. Issue #3 UPDATE. Not Q27 UNAVAILABLE-PASS. |
| `GOLD-3-PAUSED-NO-SESSION` | DLLs PRESENT **and** `Get-Process winlogon` returns nothing | Gold 3 pauses. `quser` failure is not this state. No offscreen. |

**L16a low-memory (Windows PowerShell, product tree, not the portable ZIP):**

Grok's open packet must include `$Token.C_main` and `$Token.R_main`. `git fetch origin; git checkout $Token.C_main`; `git rev-parse HEAD` must equal `$Token.C_main` or RESULT=BLOCKED. Do not run until `Get-Command cmake,ninja` succeed and Qt6 is discoverable by CMake. Missing tools → RESULT=BLOCKED, not PASS.

```powershell
$ErrorActionPreference = 'Stop'
foreach ($cmd in @('cmake','ninja','ctest')) {
  if (-not (Get-Command $cmd -ErrorAction SilentlyContinue)) { throw "BLOCKED missing $cmd" }
}
$ram = (Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory
$outJson = Join-Path $env:TEMP 'mdps-lowmem-results.json'
$head = (git rev-parse HEAD).Trim()
if ($head -ne $Token.C_main) { throw "L16a HEAD $head != C_main $($Token.C_main)" }
Write-Host "RAM_BYTES=$ram C_main=$($Token.C_main)"
cmake -S . -B build-lowmem -G Ninja -DBUILD_WITH_QT6=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
if ($LASTEXITCODE -ne 0) { throw 'lowmem configure failed' }
cmake --build build-lowmem --parallel 1
if ($LASTEXITCODE -ne 0) { throw 'lowmem build failed' }
$env:QT_QPA_PLATFORM = 'offscreen'
$env:ARCHIVE_TEST_TMP = Join-Path $PWD 'archive-test-tmp-lowmem'
$env:ARCHIVE_TEST_CONFIG_ROOT = Join-Path $PWD 'archive-test-config-lowmem'
New-Item -ItemType Directory -Force $env:ARCHIVE_TEST_TMP,$env:ARCHIVE_TEST_CONFIG_ROOT | Out-Null
ctest --test-dir build-lowmem --parallel 1 --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'lowmem ctest failed' }
@{ ram_bytes = $ram; rss_cap = 'none'; generator = 'Ninja'; result = 'PASS'; commit = $Token.C_main; ci_run = $Token.R_main } |
  ConvertTo-Json | Set-Content -LiteralPath $outJson -Encoding utf8
```

`QT_QPA_PLATFORM=offscreen` here is for CTest widgets only, **not** the portable GUI smoke. Do not skip tests. Do not commit `build-lowmem/` or `__pycache__/`.

**L16b/L16c token rule:** Luna must not run either script while any argument still contains `<` or `>`. Grok opens L16b with this hashtable filled (no placeholders). Until it exists, RESULT=BLOCKED.

```powershell
$Token = @{
  C_main = '<40-char filled by Grok>'
  R_main = '<run_id from qualification-C_main current-release.json, filled by Grok at L15>'
  PayloadSha256 = '<64-char filled by Grok>'
  ZipPath = '<filesystem path to downloaded qualification-C_main ZIP>'
  RepoRoot = '<checkout of C_main containing scripts/archive-endurance.ps1>'
  PlaylistUrl = '<operator URL archive-cli can scan>'
  VideoUrl = '<operator URL archive-cli can sync>'
  EnduranceRoot = 'C:\ArchiveHarness-Endurance'
}
```

Q25 requires working `PlaylistUrl`/`VideoUrl` (the endurance script's contract, not a YouTube hostname). If the operator cannot supply them, gold 4 **pauses**. That is not Q27 UNAVAILABLE-PASS. Do not use the CI YouTube advisory step as Q25. Do not invent a local-fixture endurance packet in this plan.

**L16b endurance (extracted Release ZIP on target Windows host), after tokens filled:**

```powershell
$ErrorActionPreference = 'Stop'
$zip = $Token.ZipPath
$expected = $Token.PayloadSha256.ToLowerInvariant()
$actual = (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actual -ne $expected) { throw "ZIP digest mismatch $actual" }
$extract = Join-Path $PWD 'extracted-release'
if (Test-Path -LiteralPath $extract) { throw 'extracted-release already exists; use a fresh directory' }
Expand-Archive -LiteralPath $zip -DestinationPath $extract -Force
$cli = @(Get-ChildItem -LiteralPath $extract -Recurse -Filter archive-cli.exe)
if ($cli.Count -ne 1) { throw "expected one archive-cli.exe, found $($cli.Count)" }
$pkg = $cli[0].Directory.FullName
if (-not (Test-Path -LiteralPath (Join-Path $pkg 'media-downloader.exe'))) { throw 'media-downloader.exe missing beside archive-cli.exe (ZIP layout)' }
$script = Join-Path $Token.RepoRoot 'scripts/archive-endurance.ps1'
if (-not (Test-Path -LiteralPath $script)) { throw "missing $script" }
New-Item -ItemType Directory -Force $Token.EnduranceRoot | Out-Null
& $script -PackageRoot $pkg -ArchiveRoot $Token.EnduranceRoot -PlaylistUrl $Token.PlaylistUrl -VideoUrl $Token.VideoUrl -DurationMinutes 60 -ExpectedCommit $Token.C_main -ExpectedCiRun $Token.R_main -OutputJson (Join-Path $Token.EnduranceRoot 'archive-endurance.json')
if ($LASTEXITCODE -ne 0) { throw 'endurance failed' }
```

DurationMinutes must stay ≥ 60.

**L16c harness:** Grok opens L16c only with a fully pasted `archive-local-harness.ps1` command (L6 names, measured tokens, no `<>`). If Grok does not paste it, L16c is not opened. If L6 is not on `C_main`, or `Select-String` still finds mandatory `ExpectedArtifactId`, L16c is BLOCKED.

### Packet L17 — Docs wave D2

Bind all current-candidate docs to `C_main` / `R_main` / payload SHA-256. Commit on a follow-up docs branch **after** qualification, merged to `main` only if docs-only. Note: a docs-only commit is **not** `C_main`. D2 must say the qualified product commit is `C_main` and the docs commit is separate. Do not retag.

After D2 lands, post issue #3: if Archive Qt6 created `qualification-C_docs`, it is a **non-gold sibling**. Do not point README/SOURCE_COMMIT at it.

### Packet L18 — Ledger QUALIFIED/INTEGRATED

Issue #3 only. List matrix §6 with run IDs. No issue #2 edits.

### Packet L19 — Grok gold audit

Luna does not perform L19. Grok walks §1.1 and §15.

---

## 11. Grok review gates

| Gate | After | Grok must verify | Opens |
|---|---|---|---|
| G0 | **FREEZE ACCEPTED** in this file; Grok explicit open | Plan matches GitHub; L1 is executable | When opened: L1, optionally L7+L8 (must not touch `archive-qt6.yml`). Not L2. Not L5. |
| G1-linux | L1 GitHub linux job | sanitizer binary exists; test 203 ran | L2 and/or L4 |
| G1-windows | L2 GitHub windows job | If smoke still throws: logs sufficient. If smoke SUCCESS: L3 is SKIP | L3 only if smoke still fails (Grok names files). No offscreen. |
| G1b | L3/L4 or L3 SKIP | Both PR jobs green on same head, tests not weakened | **G2 only.** Not L5. Not L9. |
| G2 | G1b green | YAML matches §4.3.1; no upload-artifact; one `gh release create` | L5, L6 |
| G2b | L5+L6 GitHub linux+windows SUCCESS on the **new** head | L5/L6 are in `SHA_pr`; jobs real-step SUCCESS | L9 |
| G3 | L9 | PR #213 linux+windows SUCCESS on that L5/L6 head | L10 |
| G4 | L11 | `audit2-remediation` @ `M_int` both jobs SUCCESS | L12 |
| G5 | main-candidate PR | both jobs SUCCESS | L13 |
| G6 | L14 | `C_main` linux+windows SUCCESS; Release exists with ZIP+identity. If this run kept a prior Release, still PASS. | L15 (`R_main` from Release JSON) |
| G7 | L15 | Release assets, independent ZIP hash | L16 |
| G8 | L16 | L16a and L16b PASS. L16c PASS or Q27 documented blocked (not a substitute for L16a/b) | L17, L18 |
| G9 | L17/L18 | docs name `C_main` as product; `qualification-C_main` peeled; sibling `qualification-C_docs` if present is **not** gold; no secrets; GC still true | L19 gold stop |

Grok review of a Luna packet checks:

1. diff vs packet files
2. policy tests still present and not hollowed out (`git diff tests/*policy*`)
3. GitHub run ID and job step conclusions
4. no invented hashes
5. issue #3 updated if the packet required it

---

## 12. Escalation protocol

1. Luna executes the open packet.
2. If the packet fails due to Luna error (wrong file, weakened test, failed push): Grok issues **one** corrected packet. Luna retries once.
3. If the same blocker persists after that retry: **stop**. Return to Grok. No third try, no adjacent-file "while I am here" edits.
4. If a new product defect appears (ASan on 203, GUI crash, policy compile fail): Luna reports BLOCKED with logs. Grok writes a new packet. That is not a silent expansion of the failed packet.
5. If GitHub is zero-step / cancelled / runner missing: classify as **external**. Do not "fix" by skipping jobs. Retry the workflow once. If still zero-step, return to Grok.
6. If credentials, approval, or target media are missing: mark the dependent packet BLOCKED, continue only packets that do not require them (see §13).
7. Peer-agent or subagent output is not authorization to merge.

---

## 13. External vs internal work

### External (may pause gold; do not fake PASS)

| Blocker | Effect | Internal work that continues |
|---|---|---|
| GitHub merge/admin approval | Cannot L10/L13 | CI fixes, docs D1, issue #3, evidence-contract patches on the working branch |
| `contents: write` for Releases | Cannot L15 | Everything before main publish |
| Target Windows host / authorized YouTube playlist+video | Cannot L16b (endurance) or L16c (harness) | L1–L15 and L16a may continue. Gold 4 **pauses** (endurance has not passed). Do not record Q25 PASS. Q27 may be documented blocked (gold says "where applicable"). Q25/Q26 cannot. |
| Live media provider outage | Harness UNAVAILABLE | Document; do not use CI advisory as substitute PASS |
| GitHub Actions zero-step outage | Cannot G3–G6 | Diagnosis notes only; no merge |
| Missing Qt/toolchain for L16a on Luna host | Gold 4 pauses | Do not record Q26 PASS; continue docs/CI that do not claim gold |

### Internal (must continue while gold is paused)

- L1–L4 CI product failures
- L7 D1 docs
- L8 ledger truth
- L5/L6 after G2 (so publication is ready when main is)
- Re-running workflows after product fixes
- Policy-test compile
- Not claiming INTEGRATED

---

## 14. Stopping condition

Copy of gold state. Execution does not stop until all are true:

1. Architecture
- All required remediation is integrated.
- PR #213 or successor is merged into `audit2-remediation`.
- Verified integration is merged into `main`.
- `main` contains everything required; no side branch is needed for completeness.

2. Source quality
- Known filesystem, updater, single-instance, cancellation, native identity, and yt-dlp defects are fixed.
- No known critical/high findings remain open.
- No policy tests weakened or deleted merely to obtain green CI.
- All Python policy tests compile and pass.
- Product is free of release-blocking bugs; remaining advisory gaps are explicitly documented, never silently ignored.

3. Build qualification
- Linux GitHub Actions runs real steps and succeeds.
- Windows GitHub Actions runs real steps and succeeds.
- Qt6 product builds succeed on both.
- Full test suites pass.
- Sanitizer, package, portable, recovery, and relevant runtime tests pass.

4. Extended release qualification
- Endurance testing passes.
- Low-memory testing passes.
- Portable layout and bundled tools are verified.
- Target-host/media acceptance checks pass where applicable.
- Any advisory test that cannot run in CI is executed through the local harness or explicitly documented as unavailable.

5. Release publication
- A release commit on `main` is recorded.
- A Git tag or GitHub Release identifies that exact commit.
- Portable release files have recorded SHA-256 hashes.
- Qualification evidence is committed or durably attached.
- Documentation explains what was fixed, how it was tested, and which commit is authoritative.

6. Operational hygiene
- Actions artifact retention and garbage collection work.
- Temporary artifacts are removed after each completed run.
- No repository, branch, commit, tag, or release is deleted as cleanup.
- Public repository contains no newly introduced secrets.

Do not stop at: mergeable PR, local test pass, worker saying done, a build that never ran on GitHub, green Linux with failed Windows, successful branch not on `main`, release artifact hash not tied to the qualified commit, or a red/skipped/cancelled/zero-step workflow.

---

## 15. Risks, false-done signals, and verification commands

### 15.1 Phase B (CI unblock)

**Risks:** omitting another sanitizer binary later; GUI "fixed" via offscreen; `Start-Process -RedirectStandard*` throwing on a `WIN32` binary and skipping the assertion; combining L1+L2 so a YAML error hides one diagnosis.

**False-done:** unsanitized linux green; windows CTest green; smoke step skipped; sanitizer 203 still Not Run.

**Verify:**

```text
gh run view 35586304364 --json conclusion,jobs
gh pr view 213 --json headRefOid,statusCheckRollup
gh run list --branch audit2-remediation-all-251 --workflow "Archive Qt6 qualification" --limit 3
gh run view <new-run> --log-failed
```

Linux log must contain `library-posix-native-filename` **Passed**. Windows log must contain smoke success and later seal step.

### 15.2 Phase C (evidence contract)

**Risks:** re-introducing generic artifact uploads; two `gh release create` writers; ubuntu `exit 0` on existing tag without ZIP; PowerShell JSON breaking the publication validator; `contents: write` without `actions: read`.

**False-done:** validator PASS while Release still only has compact JSON and no ZIP; linux job success with windows skipped because `needs: [linux]` lacked `if: always()`.

**Verify (PowerShell):**

```powershell
python scripts/validate-qualification-publication.py --source-root .
python tests/ci-supply-chain-policy-tests.py --source-root .
python tests/audit2-final-integration-policy-tests.py --source-root .
Select-String -Path .github/workflows/archive-qt6.yml,tests/*.py,scripts/*.py -Pattern 'upload-artifact'
```

Any `actions/upload-artifact@` in the workflow is a G2 failure.

After main: `gh release view qualification-<C_main> --json tagName,targetCommitish,assets`

### 15.3 Phase E–H (merges)

**Risks:** merging red PR; fast-forward confusion; qualifying PR head instead of `C_main`; deleting working branch.

**False-done:** GitHub "mergeable"; squash SHA undocumented; `audit2-remediation` green from an old run not `M_int`; D2 push that cancelled the in-flight main run; treating `C_docs` as `C_main`; treating the latest green main run as `R_main` when it differs from Release `current-release.json.run_id`; dropping `ExpectedCiRun`; treating `quser` failure as `GOLD-3-PAUSED-NO-SESSION`.

**Verify:**

```text
git fetch origin
git rev-parse origin/audit2-remediation origin/main origin/audit2-remediation-all-251
gh pr view 213 --json state,mergedAt,mergeCommit
gh run list --branch audit2-remediation --limit 5
gh run list --branch main --limit 5
```

### 15.4 Phase I–J (main qualification + release)

**Risks:** publish job skipped because event is not push; tag on wrong commit; clobber; hashes from a different run.

**False-done:** PR CI green reused as main qualification; compact JSON without payload SHA-256.

**Verify:**

```text
gh run list --branch main --workflow "Archive Qt6 qualification" --limit 3
gh run view <R_main> --json event,headSha,conclusion,jobs
gh release view qualification-<C_main>
Get-FileHash .\Media-Downloader-PS-Windows-Qt6-<C_main>.zip -Algorithm SHA256
```

Independent hash must equal `current-release.json.payload_sha256`.

### 15.5 Phase K (extended)

**Risks:** running endurance against an unsealed local build; using CI advisory YouTube as acceptance; skipping low-memory silently.

**False-done:** historical 1,102-iteration endurance from `4c720544`; harness PASS with wrong commit.

**Verify:** endurance JSON `commit` and `ciRunId`; harness evidence `source_commit` and `artifact_sha256`.

### 15.6 Phase L (docs/ledgers)

**Risks:** writing `C_main` into docs before it exists; editing issue #2; claiming QUALIFIED in README while Release missing.

**False-done:** D1 docs that only delete numbers and leave "final candidate passed".

**Verify (PowerShell):** `Select-String -Path docs/qualification/*.md -Pattern '4c720544'` should remain only as historical pointers; current README must name `C_main` after D2 and must not claim `origin/main` HEAD is `C_main` if D2 landed.

---

## 16. Planning critique (frozen)

**FREEZE ACCEPTED.** Terra High 1 and Terra High 2 are incorporated by reference. Gold-state §1.1/§14 remain verbatim. No offscreen smoke. No upload-artifact. No policy-test weaken. G0 may open L1 (±L7/L8) only.

---

## 17. Immediate next actions (**FREEZE ACCEPTED**)

G0 may now be opened by Grok. This freeze does not spawn Luna.

When G0 opens: **L1**, optionally **L7+L8** (docs/ledger; no `archive-qt6.yml`). Not L2. Not L5.

Then L2 after G1-linux. L5 only after G2. Merge only after G2b.

First Luna packet: **L1** (full sanitizer build + workflow `test -x` on the linux runner).

Merge/release sequence: green #213 (L5+L6 on SHA_pr) → two-parent merge on `audit2-remediation` → green integration → PR to `main` → merge `C_main` → green main jobs + Release `qualification-C_main` → `R_main` from that Release's `run_id` → endurance/low-memory PASS → docs bind to `C_main` (sibling `qualification-C_docs` is non-gold).

---

## Critique log Pass 1

Adversarial pass against the 2026-09-21 original plan. Gold-state text in §1.1 and §14 was not edited.

| ID | Severity | Finding | Plan change |
|---|---|---|---|
| P1-01 | Medium | §0.1 working tree claimed clean except pycache; the plan file is now untracked. Merge-button settings were unknown. | Re-verified 10:59Z table: untracked plan file; `allow_squash_merge=true` / `allow_rebase_merge=true`; Luna host is Windows PowerShell. Added §0.6 fact expiry. |
| P1-02 | High | L1 and L2 both edit `archive-qt6.yml` and were allowed as one push, so one job's green could be treated as Phase B done. | Serialized: G0 opens L1 only; L2 after G1-linux; combined commit/push forbidden. |
| P1-03 | High | L5 required `actions/upload-artifact@` and rewriting three policy asserts. That is a gold-forbidden policy weaken of MDPS-AUDIT2-024. | Locked G2: no upload-artifact. Windows main-only `gh release create` after polling sibling linux success. Policy forbids stay. |
| P1-04 | High | L2 used `Start-Process -RedirectStandardOutput` on a `WIN32` GUI binary and told Luna to add a Windows "virtual display". | L2: no stdout redirect; no Xvfb; `QT_ASSUME_STDERR_HAS_CONSOLE`; DLL presence logs; keep throw. |
| P1-05 | High | Repo allows squash/rebase. L13 had no `--merge`. `M_int`/`C_main` assumed different without measurement. `cancel-in-progress` can kill `R_main` if D2 pushes early. | Required `gh pr merge --merge`, two-parent proof, measure SHAs, no extra `main` pushes during G6. |
| P1-06 | High | Gold 4 requires endurance and low-memory **PASS**. Plan allowed UNAVAILABLE-as-PASS via G8 and Q26. | UNAVAILABLE pauses gold for Q25/Q26. L16 split into L16a/b/c with executable PowerShell. Advisory YouTube remains the only UNAVAILABLE-OK class. |
| P1-07 | Medium | D2 docs commit can be mistaken for the qualified product. | Added §7.1.1: tag stays on `C_main`; `origin/main` may become `C_docs`. |
| P1-08 | Medium | Luna packets used Linux `test -x`, `python3`, `rg`, and ellipsis bodies on a Windows host. L3/L12/L16 were not executable. | PowerShell local commands; workflow Linux snippets labeled runner-only; L12 title/body filled; L16 tokens documented as replacements of measured values. |
| P1-09 | Medium | False-done list missed combined L1+L2, policy-rewrite, UNAVAILABLE gold, D2 HEAD, squash merge, redirect-skip smoke. | Extended §1.4 and §15.1. |
| P1-10 | Low | Issue #3 / run `35586304364` facts still match GitHub; not stale. | Kept. Added re-fetch rule so they cannot rot. |

G0 not opened. No product/CI implementation, merge, push, or Luna spawn in this pass.

---

## Critique log Pass 2

Adversarial pass against the Pass 1 revision. Gold-state text in §1.1 and §14 was not edited. GitHub re-verified 2026-09-21T11:09Z: PR #213 still OPEN/MERGEABLE/UNSTABLE at `bebe42d`; run `35586304364` still the latest Archive Qt6 failure; issue #3 tail still `9c1caeb` at 01:10Z.

| ID | Severity | Finding | Plan change |
|---|---|---|---|
| P2-01 | High | Pass 1 G2 polled sibling linux from windows. 25 min poll < 30 min linux timeout; job-level `contents: write` drops `actions: read`; cancel-in-progress can publish then die. | Polling rejected. `windows.needs: [linux]` + `if: always() && !cancelled()`. Release step `if: main push && needs.linux.result == success`. |
| P2-02 | High | Ubuntu `publish-qualification-evidence` still `gh release create`s compact JSON and `exit 0` if tag exists — second writer, ZIP-less source of truth. | Single writer: windows `gh release create` with ZIP. Ubuntu job **verifies only**. Compact JSON stays the validator's Python snippet. |
| P2-03 | Medium | L16a/b/c still had missing-tool silence, angle-bracket tokens, and nested-ZIP `cd` comments. Not Luna-executable. | L16a preflight + results JSON. L16b/c BLOCKED until Grok fills a token table; no `<>` args; locate `archive-cli.exe` after extract. |
| P2-04 | Low | Serial L1→L2 costs an extra PR CI cycle; YAML coupling remains. | Documented cost; keep serialize; L2 must `Select-String` the L1 hunk. G0 may also open L7+L8 (no YAML). |
| P2-05 | High | Removing virtual display without a PASS path looked like an impossible GUI gate. | PASS path: GHA session exists; empty WIN32 stderr expected; L3 ordered by L2 evidence; offscreen still not pre-authorized. |
| P2-06 | Medium | `git rev-parse $sha^2` plus "revert via Grok" could smash a 108-commit squash. | `git rev-list --parents -n 1`; STOP; **no revert/reset** of shared branches. |
| P2-07 | High | L5 file list still invited editing the three upload-artifact policy tests. | Policy files not in default L5 list. No new weaken. |
| P2-08 | Medium | After L1-only G0, L7/L8 were gated on G1b for no reason. | G0 may open L1+L7+L8. L2 still after G1-linux. L3 still Grok-named files. |
| P2-09 | Low | Issue #3 / HEAD facts were 10:59Z; Pass 1 residual said issue #3 not re-fetched. | Re-fetched 11:09Z; still stale `9c1caeb` comment. CI SHAs unchanged. |
| P2-10 | Medium | Gap matrix still said "no current low-memory script" after L16a existed. §2 intro forbade serializing independent failures while packets were serialized. | Matrix + intro corrected. |

Residual for Pass 3:
- Exact L5 YAML not pasted; validator grep strings vs PowerShell ZIP attach still a footgun.
- `windows.needs: [linux]` lengthens Phase B after L5; L1/L2 currently leave jobs parallel (good for diagnosis) until L5.
- L3 still cannot be executed until L2 logs exist.
- L16b authorized URLs still external.
- `Compress-Archive` ZIP layout vs `Expand-Archive` nested folder.
- Workflow `permissions: contents: read` at top vs windows job write: fork-PR token remains read-only; same-repo PR must not hit the release `if`.
- Plan file still untracked. G0 still closed.

---

## Critique log Pass 3

Adversarial pass against the Pass 2 revision. Gold-state text in §1.1 and §14 was not edited. GitHub re-verified 2026-09-21T11:16Z: PR #213 still OPEN/MERGEABLE/UNSTABLE at `bebe42d`; run `35586304364` still latest Archive Qt6 failure.

| ID | Severity | Finding | Plan change |
|---|---|---|---|
| P3-01 | High | L5 told Luna to keep validator greps but did not paste YAML. PowerShell `ConvertTo-Json` would fail `validate-qualification-publication.py`. | §4.3.1 exact YAML: windows header; bash create step with the existing Python snippet; ubuntu verifier only; required grep list. |
| P3-02 | Medium | `windows.needs: [linux]` during L1–L4 would serialize diagnosis. G2 could open L5 before G1b. | L5 only after G1b. L1–L4 must not add `needs`. Sequential cost only on post-green runs. |
| P3-03 | Medium | `Compress-Archive -Path (Join-Path $out '*')` is a **flat** zip; hashing inner tool zips or a re-zip would bind the wrong digest. | §4.3.2: hash Release payload bytes only; `$pkg` is the dir with both exes; one `archive-cli.exe`. |
| P3-04 | High | Job-level `contents: write` on windows would apply to same-repo PRs. | Triple `if:` on create (`push` + `refs/heads/main` + `needs.linux.result == success`). Fork tokens stay read-only. L1–L4 must not add write. |
| P3-05 | Medium | L16b missing URLs vs gold 4 endurance PASS was still easy to file as Q27-style UNAVAILABLE. | External URLs pause gold 4 (Q25). Internal L1–L15/L16a continue. Q25/Q26 cannot be PASS. |
| P3-06 | Low | Untracked plan: commit-now retriggers CI; never-commit loses it for Luna. | Stay untracked through critique. L7 first `git add`. Header remains non-evidence. |
| P3-07 | High | Remaining weaken paths: edit validator for PowerShell JSON; `needs` in L1–L4; PR `gh release create`; wrong zip hash. | Added to §0.4 forbidden list. Policy test files still not in L5 default list. |
| P3-08 | Medium | L6/L9 still had placeholders; L16a wrote under `docs/qualification/`. | L6 exact param replacement; L9 `gh run list` filtered by head SHA; L16a JSON in `$env:TEMP`. |
| P3-09 | Low | GitHub facts unchanged since Pass 2. | Timestamp 11:16Z. |
| P3-10 | Medium | Sequence diagram still treated ubuntu publish as creator; D1 did not list this plan file. | Diagram = windows create + ubuntu verify. D1/L7 commit the plan as non-evidence. |

Residuals for Terra High (Pass 4):
- windows-2022 Git Bash `python` vs `python3` in the L5 create step.
- `gh release view --json targetCommitish` is not used (tag name only); confirm tag `--target` binds `C_main`.
- L3 still file-TBD until L2 logs.
- L16b URLs still operator-supplied.
- Sanitizer 30-minute timeout after full build.
- G0 still closed. Do not implement.

---

## Critique log Pass 4

Adversarial pass against Pass 3 plus independent Terra High 1 (`docs/qualification/TERRA-HIGH-1-CRITIQUE.md`, retained). Gold-state text in §1.1 and §14 was not edited. HEAD still `bebe42d`. G0 not opened.

Terra High findings:

| Terra | Verdict | Plan change |
|---|---|---|
| H1 G1b opens L9 before L5 on SHA_pr | **Accepted** | G1b opens G2/L5+L6 only. New **G2b** after L5+L6 CI green opens L9. Merge before L5/L6 is forbidden/false-done. |
| H2 Git Bash `python` | **Accepted** | `PYBIN="$(command -v python.exe \|\| python3 \|\| python)"`; PR-safe proof step; grepped snippet unchanged; validator not edited. |
| H3 endurance script not in ZIP | **Accepted** | L16b runs `scripts/archive-endurance.ps1` from `$Token.RepoRoot` at `C_main`. Do not add it to the portable ZIP without requalify. |
| H4 `cmp` vs Compress-Archive | **Accepted** | No ZIP `cmp` on retry. First create is payload. Keep prior if commit+digest match. No `--clobber`. |
| H5 D2 publishes `qualification-C_docs` | **Accepted option 3** (not 1: would fail gold 1; not 2: extra YAML before G0). Sibling Release allowed; gold identity is only `qualification-C_main`; G9 fails if docs name `C_docs` as product. |
| H6 asset list vs create argv | **Accepted** | Top-level assets: compact JSON, ZIP, `current-release.json` only. SHA256SUMS inside ZIP. |
| H7 targetCommitish | **Accepted** | Keep `--target $GITHUB_SHA`. Peel `git/ref/tags`. Do not assert `targetCommitish == sha`. |
| M1 L3 offscreen magnet | **Accepted** | L3 only if smoke still throws; delete session-setup candidate; no offscreen. |
| M2 Q25 vs YouTube | **Accepted (a)** | Q25 needs operator URLs the script can scan; missing pauses gold 4. No local-fixture packet. No UNAVAILABLE-PASS. Rejected using CI advisory as Q25. |
| M3 L16a HEAD | **Accepted** | Must checkout `C_main`. |
| M4 validator table | **Accepted** | Quoted real greps; D2 must keep `qualification-<full-source-commit>` in the report. |
| M5 L6 ExpectedArtifactId | **Accepted** | Param, evidence JSON, integration tests, zero matches, GitHub CTest gate. |
| M6 tokens | **Accepted** | Hashtable template; L15 no `<>`. |
| M7 BOM | **Accepted** | `[IO.File]::WriteAllText`. |
| M8 run_id types / retry | **Accepted** | `str()` compare; retry may keep prior `run_id`. |
| M9 sanitizer timeout | **Accepted** | Missing target is small; stall → split job, not drop tests. |
| L1 line numbers | **Accepted** | 564–575. |
| L3 tree dump | **Accepted** | Write tree to file; Host only DLL loop. |

**Rejected (would weaken gold / policy / GUI):** offscreen smoke; upload-artifact; validator rewrite; UNAVAILABLE-as-PASS for Q25/Q26; adding endurance script to ZIP without requalify; `--clobber`; merging #213 on pre-L5 green.

Residuals for Pass 5:
- L3 files still TBD until L2 logs; no session + DLLs present still has no authorized pass path (fail-closed).
- First G6 is first real `gh release create`.
- Operator URLs and Qt/cmake for L16.
- Same-repo PR `contents: write` after L5.
- `qualification-C_docs` sibling after D2 must not win G9.

---

## Critique log Pass 5

Final Grok self-critique. Gold-state §1.1 and §14 not edited. GitHub re-verified 2026-09-21T11:45Z: PR #213 still OPEN/MERGEABLE/UNSTABLE at `bebe42d`. G0 not opened. Terra High 1 file retained. Terra High 2 still due.

| ID | Severity | Finding | Plan change |
|---|---|---|---|
| P5-01 | High | “No authorized GUI pass path” overstated gold 3 as impossible. Item 4 (no session) is not the default. | Gold 3 reachable via packaging/init (L2 session probe expected to see winlogon). Item 4 is named `GOLD-3-PAUSED-NO-SESSION` only. No offscreen. |
| P5-02 | High | First G6 was first real publish Python/ZIP check. | PR-only dry-run: PYBIN + write compact JSON + verify `payload_sha256`; still exactly one `gh release create` (main-only). |
| P5-03 | Medium | Same-repo PR `contents: write` after L5. | Keep write (create needs it). L5 PR acceptance: `gh release list` must not contain `qualification-<PR-head>`. Leak = STOP. |
| P5-04 | Medium | `qualification-C_docs` could win G9 by silence. | G9 requires docs name `C_main`; L17 posts sibling-non-gold note. |
| P5-05 | Medium | L16 missing tools/URLs could be filed as skip. | Named `GOLD-4-PAUSED-LOWMEM` / `GOLD-4-PAUSED-ENDURANCE`. BLOCKED ≠ PASS. |
| P5-06 | Low | L16c still not independently executable. | Not opened unless Grok pastes full argv. |
| P5-07 | High | Recheck weaken paths. | Still forbidden: offscreen smoke, upload-artifact, policy rewrites, squash, merge-before-L5. None reintroduced. |
| P5-08 | Low | Working tree omitted Terra critique file. | §0.1 lists it untracked. L7 commits both planning files. |

**Rejected this pass:** skip-guard YAML to suppress `qualification-C_docs` (Terra option 2) — extra workflow before G0. Offscreen as gold-3 escape. upload-artifact to test publish on PR.

Residuals for Terra High 2:
- L3 file list still TBD until L2 logs (by design).
- Create step still unexecuted until G6; PR dry-run covers Python/ZIP, not `gh release create` itself.
- Operator URLs / Qt remain external BLOCKED paths.
- Job-level `contents: write` on same-repo PRs remains; `if:` + release-list check are the guards.
- G0 closed until Terra High 2 freeze.

---

## Critique log Terra High 2

Independent critique: `docs/qualification/TERRA-HIGH-2-CRITIQUE.md` (do not delete). Gold-state §1.1/§14 not edited by Terra. Incorporated here:

| Terra 2 | Verdict | Plan change |
|---|---|---|
| H1 `R_main` vs first-publish `run_id` | **Accepted** | `R_main` := Release `current-release.json.run_id`. L15 copies it. L16b `-ExpectedCiRun` uses it. Do not drop the check. G6 PASS if prior Release kept. |
| H2 `quser` ≠ no-session | **Accepted** | Pause only if DLLs PRESENT **and** `winlogon` absent. `quser` is noise. |
| M1 leak SHA | **Accepted** | Check both `headRefOid` and job `GITHUB_SHA`. Ambient token overclaim corrected. |
| M2 §4.3 item 5 `cmp` | **Accepted** | Item 5 now matches §7.3 (prior ZIP vs prior identity). |
| M3 G1b Opens L5 | **Accepted** | G1b Opens **G2 only**. |
| M4 L6 fixture tag | **Accepted** | `-ExpectedReleaseTag ('qualification-' + commit)` with `commit = 'a'*40`. |
| M5 `gh` on PR | **Accepted** | `command -v gh` + `gh --version` on PYBIN step. No PR create. |
| M6 L5 docs greps | **Accepted** | Keep `qualification-<full-source-commit>` / `publish-qualification-evidence` / `ordinary Actions artifact expiry`. |
| L1 Terra 2 in L7 | **Accepted** | L7 `git add`s TERRA-HIGH-2-CRITIQUE.md. |
| L2 L15 origin/main | **Accepted** | `$Cmain = $Token.C_main`. |

**Rejected:** dropping `ExpectedCiRun`; offscreen GUI; PR `gh release create` / `--draft` / upload-artifact; skip-guard YAML for `C_docs`.

---

## Critique log Final freeze

**FREEZE ACCEPTED** 2026-09-21T11:59Z. HEAD `bebe42d`. PR #213 still OPEN/UNSTABLE. Gold-state verbatim. Five Grok critiques + two Terra High critiques incorporated. G0 may open L1 (±L7/L8). This freeze does not spawn Luna and does not implement product/CI.

Must-remain: no offscreen smoke; no upload-artifact; no policy-test weaken; no squash; no merge-before-L5; `R_main` from first-publish Release JSON; `winlogon` (not `quser`) for gold-3 pause; endurance script from `C_main` checkout; three top-level Release assets; tag peel; sibling `qualification-C_docs` is non-gold.

---

## Packet L3 OPEN (G1-windows)

Opened 2026-09-21 from L2 evidence run `35599188261` / `d8c2081`. Path 3 (product init / single-instance). Not missing DLL. Not `GOLD-3-PAUSED-NO-SESSION`.

Executable packet: `docs/qualification/L3-PACKET.md`.

Exact files Luna may edit: `src/utils/single_instance.hpp`, `src/engines.cpp` (`socketPath()`), `src/engines.h` only if the signature must change; policy tests only to add stricter asserts. No YAML smoke weaken. No `themes.cpp` / `mainwindow.cpp` in this packet. Architect did not implement the fix.

---

## Packet L3b OPEN (not G1b)

L3 smoke PASS on run `35601901658` / `892be00`. Windows job still FAILED on **GUI settings preserve package seal**. Not G1b. Not G2. Not merge.

Executable packet: `docs/qualification/L3B-PACKET.md`.

Exact files: `tests/archive-gui-package-immutability.ps1` (required). Cause: `pass=($settingsExists -and ...)` contradicts the line-28 isolation throw. Keep the test; invert `pass` to `-not $settingsExists` plus seal scans. Do not skip the step.

---

## G1b PASS / G2 OPEN

Verified run `35603460222` head `1039f73f97420fb40ac160212819c856a2b0e650`: linux SUCCESS, windows SUCCESS (smoke + seal + identity). Publish skipped (PR). **G1b PASS.**

G2 OPEN: `docs/qualification/L5-PACKET.md` + `docs/qualification/L6-PACKET.md`. Not L9. Not merge. `SHA_pr` for L10 must contain L5+L6. Architect did not implement YAML.

---

## Packet L5-RETRY OPEN (not L5-FIX, not L9)

Run `35605481136` / `09079242`: linux SUCCESS; windows Assemble FAIL on GitHub **500 HTML** for pinned deno zip (not SHA mismatch, not 404). Packet: `docs/qualification/L5-RETRY-PACKET.md`. `gh run rerun 35605481136 --failed`. No YAML edit.

---

## G2b PASS / L10 OPEN

Rerun attempt 2 of `35605481136` on `09079242`: linux SUCCESS, windows SUCCESS, dry-run PASS, publish create SKIPPED, no `qualification-*` releases. PR #213 MERGEABLE/CLEAN. Packet: `docs/qualification/L10-PACKET.md`. Architect did not merge. Not `main`. Not gold.

---

## G4 PASS / L12 OPEN

#213 MERGED. `M_int=5bbb9cb` two-parent. Run `35609113575` push SUCCESS linux+windows real steps. Packet: `docs/qualification/L12-PACKET.md`. Not L13. Not `C_main`. Architect did not open the main PR.

---

## G5 PASS / L13 OPEN

PR #214 run `35611200930` pull_request SUCCESS linux+windows. `origin/main` still `8a62c9b`. Packet: `docs/qualification/L13-PACKET.md`. Architect did not merge. Not gold until G6 `qualification-C_main` ZIP.

---

## G6 incomplete / L15 OPEN

`C_main=bb949284`. Run `35613369266`: linux+windows SUCCESS, Release PRESENT, ubuntu verifier FAIL (`gh release view` without `GH_REPO`). Packet: `docs/qualification/L15-PACKET.md`. Do not clobber. Do not claim gold. L16 closed.

---

## G6 product identity accepted / L16 OPEN

Verifier PASS run `35617860561` / `C_verify=77caa99`. ZIP digest on `qualification-bb949284…` unchanged. `R_main=35613369266`. Packet: `docs/qualification/L16-PACKET.md`. Not full gold. Sibling `qualification-77caa99` is non-gold.

---

## L16 returned / L17 + GOLD-4-PAUSED OPEN

L16a `GOLD-4-PAUSED-LOWMEM`. L16b `GOLD-4-PAUSED-ENDURANCE`. L16c not opened. Packets: `docs/qualification/L17-PACKET.md`, `docs/qualification/GOLD-4-PAUSED-PACKET.md`. Gold incomplete. L19 closed. Sibling `qualification-77caa99` must not win G9.
