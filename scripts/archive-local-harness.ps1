param(
    [Parameter(Mandatory=$true)][string]$ArchiveRoot,
    [Parameter(Mandatory=$true)][string]$PlaylistUrl,
    [Parameter(Mandatory=$true)][string]$VideoUrl,
    [Parameter(Mandatory=$true)][ValidatePattern('^[0-9a-f]{40}
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Bind this extracted directory to the externally trusted GitHub artifact
# before executing any candidate binary. The expected digest/identity values
# must come from GitHub (or another trusted handoff), never from this package.
$artifactZip = [IO.Path]::GetFullPath($ArtifactZipPath)
if (!(Test-Path -LiteralPath $artifactZip -PathType Leaf)) { throw 'Externally anchored artifact ZIP is missing' }
$actualArtifactSha256 = (Get-FileHash -LiteralPath $artifactZip -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actualArtifactSha256 -ne $ExpectedArtifactSha256.ToLowerInvariant()) { throw 'External artifact SHA-256 does not match the trusted digest' }

Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = [IO.Compression.ZipFile]::OpenRead($artifactZip)
try {
    $fileEntries = @($zip.Entries | Where-Object { $_.Name })
    $identityEntries = @($fileEntries | Where-Object { $_.FullName.Replace('\','/') -match '(^|/)build-identity\.json
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or
    [string]$identity.run_id -ne $ExpectedRunId -or
    [string]$identity.repository -ne $ExpectedRepository -or
    $identity.qualification -ne 'windows-ci-qualified-for-local-harness') {
    throw 'Candidate repository, run, commit or qualification does not match the trusted artifact identity'
}
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    repository=$ExpectedRepository; source_commit=$ExpectedCommit; ci_run_id=$ExpectedRunId;
    artifact_id=$ExpectedArtifactId; artifact_sha256=$actualArtifactSha256; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedCommit,
    [Parameter(Mandatory=$true)][ValidatePattern('^[0-9a-fA-F]{64}
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedArtifactSha256,
    [Parameter(Mandatory=$true)][ValidatePattern('^[0-9]+
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedArtifactId,
    [Parameter(Mandatory=$true)][ValidatePattern('^[0-9]+
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedRunId,
    [Parameter(Mandatory=$true)][ValidatePattern('^[^/\\s]+/[^/\\s]+
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedRepository,
    [Parameter(Mandatory=$true)][string]$ArtifactZipPath,
    [switch]$AllowExistingArchive
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
 })
    if ($identityEntries.Count -ne 1) { throw 'Artifact ZIP must contain exactly one build-identity.json' }
    $identityName = $identityEntries[0].FullName.Replace('\','/')
    $prefix = $identityName.Substring(0, $identityName.Length - 'build-identity.json'.Length)
    $zipListed = @{}
    foreach ($entry in $fileEntries) {
        $normalized = $entry.FullName.Replace('\','/')
        if (!$normalized.StartsWith($prefix, [StringComparison]::Ordinal)) { continue }
        $relative = $normalized.Substring($prefix.Length)
        if (!$relative) { continue }
        if ($zipListed.ContainsKey($relative)) { throw "Duplicate artifact ZIP path: $relative" }
        $path = Safe-Child $here $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf)) { throw "Extracted package is missing trusted artifact file: $relative" }
        $stream = $entry.Open()
        try {
            $sha = [Security.Cryptography.SHA256]::Create()
            try { $entryHash = ([BitConverter]::ToString($sha.ComputeHash($stream))).Replace('-','').ToLowerInvariant() }
            finally { $sha.Dispose() }
        } finally { $stream.Dispose() }
        $extractedHash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($entryHash -ne $extractedHash) { throw "Extracted package differs from externally anchored artifact: $relative" }
        $zipListed[$relative] = $entryHash
    }
    foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
        $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
        if (!$zipListed.ContainsKey($relative)) { throw "Extracted package contains a file not present in the trusted artifact ZIP: $relative" }
    }
} finally {
    $zip.Dispose()
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedCommit,
    [Parameter(Mandatory=$true)][ValidatePattern('^[0-9a-fA-F]{64}
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedArtifactSha256,
    [Parameter(Mandatory=$true)][ValidatePattern('^[0-9]+
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedArtifactId,
    [Parameter(Mandatory=$true)][ValidatePattern('^[0-9]+
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedRunId,
    [Parameter(Mandatory=$true)][ValidatePattern('^[^/\\s]+/[^/\\s]+
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
)][string]$ExpectedRepository,
    [Parameter(Mandatory=$true)][string]$ArtifactZipPath,
    [switch]$AllowExistingArchive
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$here = [IO.Path]::GetFullPath((Split-Path -Parent $MyInvocation.MyCommand.Path))
$root = [IO.Path]::GetFullPath($ArchiveRoot)
$cli = Join-Path $here 'archive-cli.exe'
$identityPath = Join-Path $here 'build-identity.json'
$sumPath = Join-Path $here 'SHA256SUMS.txt'

function Get-ReparseTag([string]$path) {
    $output = @(& fsutil.exe reparsepoint query $path 2>$null)
    if ($LASTEXITCODE -ne 0) { return $null }
    foreach ($line in $output) {
        if ([string]$line -match 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') {
            return [Convert]::ToUInt32($Matches[1], 16)
        }
    }
    return $null
}
function Is-CloudPlaceholderTag([uint32]$tag) {
    # Microsoft Cloud Files uses the documented 0x9000n01A tag family.
    $cloudMask = [Convert]::ToUInt32('FFFF0FFF', 16)
    $cloudBase = [Convert]::ToUInt32('9000001A', 16)
    return (($tag -band $cloudMask) -eq $cloudBase)
}
function Safe-Child([string]$parent, [string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative -match '[:\\]' -or $relative -match '(^|/)\.{1,2}(/|$)' -or $relative -match '[\x00-\x1f]') { throw "Unsafe relative path: $relative" }
    $prefix = $parent.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $path = [IO.Path]::GetFullPath((Join-Path $parent $relative))
    if (!$path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { throw "Path escapes root: $relative" }
    $cursor = $path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $attributes = (Get-Item -LiteralPath $cursor -Force).Attributes
            if (($attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                $tag = Get-ReparseTag $cursor
                if ($null -eq $tag -or -not (Is-CloudPlaceholderTag $tag)) { throw "Linked path refused: $cursor" }
            }
        }
        $next = Split-Path -Parent $cursor
        if (!$next -or $next -eq $cursor) { break }; $cursor = $next
    }
    return $path
}
function Invoke-Archive([string[]]$CommandArgs) {
    $output = @(& $cli @CommandArgs 2>&1)
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    if ($exitCode -ne 0) { throw "Archive command failed: $($CommandArgs[0]), exit=$exitCode" }
    $values = @{}
    foreach ($line in $output) { if ([string]$line -match '^([a-z_]+)=(.*)$') { $values[$Matches[1]] = $Matches[2] } }
    return $values
}
function Media-Hashes($values) {
    $hashes = @{}
    foreach ($kind in @('video','audio')) {
        $relative = [string]$values["${kind}_path"]
        if (!$relative -or $relative -notmatch ('^' + $kind + '/')) { throw "Missing or unsafe $kind path" }
        $path = Safe-Child $root $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -le 0) { throw "Missing or empty canonical media: $relative" }
        $hashes[$relative] = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

# Validate identity and every declared package byte before executing any binary.
foreach ($name in @('archive-cli.exe','build-identity.json','SHA256SUMS.txt')) { $null = Safe-Child $here $name }
if (!(Test-Path -LiteralPath $cli) -or !(Test-Path -LiteralPath $identityPath) -or !(Test-Path -LiteralPath $sumPath)) { throw 'Portable candidate is incomplete' }
$identity = Get-Content -LiteralPath $identityPath -Raw | ConvertFrom-Json
if ($identity.commit -ne $ExpectedCommit -or $identity.qualification -ne 'windows-ci-qualified-for-local-harness') { throw 'Candidate commit or qualification does not match the expected CI build' }
$listed = @{}
foreach ($line in Get-Content -LiteralPath $sumPath) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw 'Malformed SHA256SUMS entry' }
    $expected = $Matches[1].ToLowerInvariant(); $relative = $Matches[2]
    if ($listed.ContainsKey($relative)) { throw "Duplicate manifest path: $relative" }
    $path = Safe-Child $here $relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Package hash mismatch: $relative" }
    $listed[$relative] = $expected
}
foreach ($required in @('archive-cli.exe','archive-local-harness.ps1','build-identity.json','bin/yt-dlp.exe','bin/deno.exe','3rdParty/ffmpeg/bin/ffmpeg.exe','3rdParty/ffmpeg/bin/ffprobe.exe','PORTABLE_MANIFEST.txt')) {
    if (!$listed.ContainsKey($required)) { throw "Required package file is not sealed: $required" }
}
foreach ($file in Get-ChildItem -LiteralPath $here -Recurse -File -Force) {
    $relative = $file.FullName.Substring($here.TrimEnd('\','/').Length + 1).Replace('\','/')
    if ($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)) { throw "Unsealed package file: $relative. Use a fresh extraction." }
}
if ($root.StartsWith($here.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or $root -eq $here) { throw 'Archive Root must be outside the immutable portable package' }
if ((Test-Path -LiteralPath $root) -and !$AllowExistingArchive -and @(Get-ChildItem -LiteralPath $root -Force).Count -ne 0) { throw 'Use an empty isolated Archive Root, or explicitly pass -AllowExistingArchive' }

$started = [DateTime]::UtcNow.ToString('o')
$null = Invoke-Archive -CommandArgs @('preflight', $root)
$scan = Invoke-Archive -CommandArgs @('scan', $root, $PlaylistUrl, 'Local harness playlist')
if ($scan['complete'] -ne 'true' -or [int]$scan['observed'] -lt 1) { throw 'Playlist discovery was not complete and nonempty' }
$sync = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verify = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
if ($sync['sync'] -ne 'PASS' -or $verify['verify'] -ne 'PASS' -or !$sync['item_key'] -or $verify['item_key'] -ne $sync['item_key']) { throw 'Exact requested item did not verify' }
$before = Media-Hashes $verify
$repeat = Invoke-Archive -CommandArgs @('sync-item', $root, $VideoUrl)
$verifiedAgain = Invoke-Archive -CommandArgs @('verify-item', $root, $VideoUrl)
$after = Media-Hashes $verifiedAgain
if ($repeat['sync'] -ne 'PASS' -or $verifiedAgain['verify'] -ne 'PASS' -or $verifiedAgain['item_key'] -ne $verify['item_key'] -or $before.Count -ne $after.Count) { throw 'Idempotent rerun failed' }
foreach ($path in $before.Keys) { if (!$after.ContainsKey($path) -or $before[$path] -ne $after[$path]) { throw "Rerun changed canonical media: $path" } }
$evidence = @{
    schema_version=1; result='PASS'; started_at=$started; completed_at=[DateTime]::UtcNow.ToString('o');
    source_commit=$ExpectedCommit; ci_run_id=$identity.run_id; item_key=$verify['item_key'];
    observed=[int]$scan['observed']; verified_media=$after; rerun='identical-canonical-media';
    manifest_sha256=(Get-FileHash -LiteralPath $sumPath -Algorithm SHA256).Hash.ToLowerInvariant()
}
$evidencePath = Safe-Child $root ('local-harness-evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffffff') + '.json')
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Host "Local Archive harness PASS: $($verify['item_key'])"
Write-Host "Evidence: $evidencePath"
