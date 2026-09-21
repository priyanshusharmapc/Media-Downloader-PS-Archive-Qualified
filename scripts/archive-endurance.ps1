#Requires -Version 5.1
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$PackageRoot,
    [Parameter(Mandatory=$true)][string]$ArchiveRoot,
    [Parameter(Mandatory=$true)][string]$PlaylistUrl,
    [Parameter(Mandatory=$true)][string]$VideoUrl,
    [int]$DurationMinutes = 60,
    [string]$PlanPath,
    [string]$BasePackageRoot,
    [Parameter(Mandatory=$true)][string]$ExpectedCommit,
    [Parameter(Mandatory=$true)][string]$ExpectedCiRun,
    [string[]]$AllowedOverlayPath = @(),
    [string[]]$AllowedOverlaySha256 = @(),
    [switch]$SkipPreflight,
    [string]$OutputJson
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
if($DurationMinutes -lt 60){throw 'Endurance qualification requires at least 60 minutes.'}

$cli=Join-Path ([IO.Path]::GetFullPath($PackageRoot)) 'archive-cli.exe'
if(!(Test-Path -LiteralPath $cli -PathType Leaf)){throw "archive-cli.exe not found: $cli"}
$root=[IO.Path]::GetFullPath($ArchiveRoot)
$parent=Split-Path -Parent $root
New-Item -ItemType Directory -Path $root -Force|Out-Null
if([string]::IsNullOrWhiteSpace($OutputJson)){$OutputJson=Join-Path $parent 'archive-endurance.json'}
if($PlanPath){$env:ARCHIVE_TEST_PLAN=[IO.Path]::GetFullPath($PlanPath)}
$basePackage=[IO.Path]::GetFullPath($(if($BasePackageRoot){$BasePackageRoot}else{$PackageRoot}))

function Assert-PackageSeal([string]$path){
    $identityPath=Join-Path $path 'build-identity.json'
    if(!(Test-Path -LiteralPath $identityPath -PathType Leaf)){throw "Missing build identity: $identityPath"}
    $identity=Get-Content -LiteralPath $identityPath -Raw|ConvertFrom-Json
    if($ExpectedCommit -and $identity.commit -ne $ExpectedCommit){throw "Unexpected package commit: $($identity.commit)"}
    if($ExpectedCiRun -and [string]$identity.run_id -ne [string]$ExpectedCiRun){throw "Unexpected package CI run: $($identity.run_id)"}
    $manifest=Join-Path $path 'SHA256SUMS.txt';if(!(Test-Path -LiteralPath $manifest -PathType Leaf)){throw "Missing package manifest: $manifest"}
    $entries=@();foreach($line in Get-Content -LiteralPath $manifest){if($line -match '^([0-9a-fA-F]{64})  (.+)$'){$entries += [pscustomobject]@{Expected=$Matches[1].ToLowerInvariant();Path=$Matches[2]}}}
    if(!$entries.Count){throw 'Package manifest is empty'}
    $listed=@{};$mismatches=@();$missing=@();foreach($entry in $entries){$listed[$entry.Path]=$true;$file=Join-Path $path ($entry.Path.Replace('/','\'));if(!(Test-Path -LiteralPath $file -PathType Leaf)){$missing+=$entry.Path}elseif((Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Expected){$mismatches+=$entry.Path}}
    $unsealed=@();foreach($file in Get-ChildItem -LiteralPath $path -Recurse -File -Force){$relative=$file.FullName.Substring($path.Length+1).Replace('\','/');if($relative -ne 'SHA256SUMS.txt' -and !$listed.ContainsKey($relative)){$unsealed+=$relative}}
    if(($missing.Count -gt 0) -or ($mismatches.Count -gt 0) -or ($unsealed.Count -gt 0)){throw "Base package seal invalid: missing=$($missing.Count) mismatches=$($mismatches.Count) unsealed=$($unsealed.Count)"}
    return [ordered]@{root=$path;commit=$identity.commit;ciRunId=[string]$identity.run_id;manifestEntries=$entries.Count;manifestSha256=(Get-FileHash -LiteralPath $manifest -Algorithm SHA256).Hash.ToLowerInvariant();unsealed=$unsealed;entries=$entries}
}

function Assert-ExecutionPackage([string]$base,[string]$execution,$seal,[string[]]$allowedOverlay,[string[]]$allowedOverlaySha256){
    # Every sealed file remains immutable. Overlay additions require both an
    # explicit relative path and an independently supplied SHA-256 digest.
    $listed=@{}
    foreach($entry in $seal.entries){
        $listed[[string]$entry.Path]=[string]$entry.Expected
    }

    $overlayHashes=@{}
    foreach($spec in $allowedOverlaySha256){
        if([string]::IsNullOrWhiteSpace($spec)){continue}
        $separator=$spec.LastIndexOf('=')
        if($separator -le 0){throw "Overlay SHA-256 must use relative/path=<sha256>: $spec"}
        $relative=$spec.Substring(0,$separator).Replace('\','/').TrimStart('/')
        $expected=$spec.Substring($separator+1).ToLowerInvariant()
        if($expected -notmatch '^[0-9a-f]{64}$'){throw "Invalid overlay SHA-256 for $relative"}
        $overlayHashes[$relative]=$expected
    }

    $allowed=@{}
    foreach($item in $allowedOverlay){
        if([string]::IsNullOrWhiteSpace($item)){continue}
        $relative=$item.Replace('\','/').TrimStart('/')
        if($relative.Contains('../') -or $relative -eq '..'){throw "Unsafe allowed overlay path: $item"}
        if($listed.ContainsKey($relative)){throw "Allowed overlay cannot replace sealed package path: $relative"}
        if(!$overlayHashes.ContainsKey($relative)){throw "Allowed overlay is missing expected SHA-256: $relative"}
        $allowed[$relative]=$true
    }
    foreach($relative in $overlayHashes.Keys){
        if(!$allowed.ContainsKey($relative)){throw "Overlay SHA-256 supplied for undeclared path: $relative"}
    }

    $differences=@()
    foreach($entry in $seal.entries){
        $relative=[string]$entry.Path
        $file=Join-Path $execution ($relative.Replace('/','\'))
        if(!(Test-Path -LiteralPath $file -PathType Leaf)){
            $differences+="missing:$relative"
            continue
        }
        $actual=(Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant()
        if($actual -ne [string]$entry.Expected){$differences+="modified:$relative"}
    }

    foreach($relative in $allowed.Keys){
        $file=Join-Path $execution ($relative.Replace('/','\'))
        if(!(Test-Path -LiteralPath $file -PathType Leaf)){
            $differences+="missing-overlay:$relative"
            continue
        }
        $actual=(Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant()
        if($actual -ne [string]$overlayHashes[$relative]){$differences+="overlay-hash-mismatch:$relative"}
    }

    foreach($file in Get-ChildItem -LiteralPath $execution -Recurse -File -Force){
        $relative=$file.FullName.Substring($execution.Length+1).Replace('\','/')
        if($relative -eq 'SHA256SUMS.txt'){continue}
        if(!$listed.ContainsKey($relative) -and !$allowed.ContainsKey($relative)){
            $differences+="unsealed:$relative"
        }
    }

    if($differences.Count -gt 0){
        throw "Execution package differs from sealed base outside declared overlay: $($differences -join ', ')"
    }

    $cliRelative='archive-cli.exe'
    if(!$listed.ContainsKey($cliRelative)){throw "Sealed package manifest does not contain archive-cli.exe"}
    $cliPath=Join-Path $execution $cliRelative
    $actualCli=(Get-FileHash -LiteralPath $cliPath -Algorithm SHA256).Hash.ToLowerInvariant()
    $expectedCli=[string]$listed[$cliRelative]
    if($actualCli -ne $expectedCli){throw "Executed archive-cli.exe does not match sealed package"}

    return [ordered]@{
        root=$execution
        allowedOverlay=@($allowed.Keys | Sort-Object)
        overlaySha256=$overlayHashes
        expectedArchiveCliSha256=$expectedCli
        archiveCliSha256=$actualCli
    }
}

$packageSeal=Assert-PackageSeal $basePackage
$executionSeal=Assert-ExecutionPackage $basePackage ([IO.Path]::GetFullPath($PackageRoot)) $packageSeal $AllowedOverlayPath $AllowedOverlaySha256
$packageFfmpeg=Join-Path $PackageRoot '3rdParty\ffmpeg\bin'
if(Test-Path -LiteralPath $packageFfmpeg -PathType Container){$env:PATH=$packageFfmpeg+';'+$env:PATH}

function Invoke-Archive([string[]]$Arguments){
    $output=& $cli @Arguments 2>&1
    [pscustomobject]@{Args=$Arguments;ExitCode=$LASTEXITCODE;Output=($output -join "`n");At=(Get-Date).ToString('o')}
}
function Get-MediaHashes{
    $hashes=[ordered]@{}
    foreach($directory in @('Video','Audio')){
        $path=Join-Path $root $directory
        if(Test-Path -LiteralPath $path){
            foreach($file in Get-ChildItem -LiteralPath $path -File -Recurse){
                $relative=$file.FullName.Substring($root.Length).TrimStart('\','/')
                $hashes[$relative]=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
            }
        }
    }
    return $hashes
}

$start=Get-Date
$preflight=$null
if(!$SkipPreflight){
    $preflight=Invoke-Archive @('preflight',$root)
    if($preflight.ExitCode -ne 0){throw $preflight.Output}
}
$deadline=(Get-Date).AddMinutes($DurationMinutes)
$iterations=0;$failures=@();$baseline=$null;$maxMediaCount=0;$mediaChanges=@()
while((Get-Date)-lt $deadline){
    $iterations++
    foreach($arguments in @(
        @('scan',$root,$PlaylistUrl,'Endurance qualification'),
        @('sync-item',$root,$VideoUrl),
        @('verify-item',$root,$VideoUrl)
    )){
        $result=Invoke-Archive $arguments
        if($result.ExitCode -ne 0){$failures+=,$result;break}
    }
    if($failures.Count -gt 0){break}
    $hashes=Get-MediaHashes
    $maxMediaCount=[Math]::Max($maxMediaCount,$hashes.Count)
    if($null -eq $baseline){$baseline=$hashes}else{
        foreach($key in $baseline.Keys){
            if(!$hashes.Contains($key) -or $hashes[$key] -ne $baseline[$key]){$mediaChanges+=$key}
        }
        foreach($key in $hashes.Keys){if(!$baseline.Contains($key)){$mediaChanges+=$key}}
        if($mediaChanges.Count -gt 0){$failures+=,[pscustomobject]@{Args=@('hash-stability');ExitCode=1;Output=('Media set/hash changed: '+($mediaChanges -join ', '));At=(Get-Date).ToString('o')}}
    }
    if($failures.Count -gt 0){break}
    if(($iterations%50)-eq 0){Write-Host ("Endurance iteration {0}; remaining {1:n0} seconds" -f $iterations,[Math]::Max(0,($deadline-(Get-Date)).TotalSeconds))}
}

$tempEntries=if(Test-Path -LiteralPath (Join-Path $root 'Temp')){@(Get-ChildItem -LiteralPath (Join-Path $root 'Temp') -Force -Recurse).Count}else{0}
$journal=Test-Path -LiteralPath (Join-Path $root 'State\ArchiveMode\transaction.json')
$report=[ordered]@{
    schema_version=1
    started=$start.ToString('o')
    ended=(Get-Date).ToString('o')
    requested_minutes=$DurationMinutes
    iterations=$iterations
    failures=$failures
    preflightSkipped=[bool]$SkipPreflight
    preflightReason=if($SkipPreflight){'deliberate fake-provider overlay; exact sealed base package was manifest-validated and preflighted separately'}else{$null}
    packageSeal=$packageSeal
    executionSeal=$executionSeal
    basePackageRoot=$basePackage
    packageRoot=[IO.Path]::GetFullPath($PackageRoot)
    expectedCommit=$ExpectedCommit
    expectedCiRun=[string]$ExpectedCiRun
    media_hashes=$baseline
    media_changes=$mediaChanges
    max_media_count=$maxMediaCount
    temp_entries=$tempEntries
    transaction_journal_present=$journal
    pass=($failures.Count -eq 0 -and (Get-Date)-ge $deadline -and !$journal -and $packageSeal.manifestEntries -gt 0 -and $executionSeal.archiveCliSha256)
}
$report|ConvertTo-Json -Depth 10|Set-Content -LiteralPath $OutputJson -Encoding UTF8
if(!$report.pass){throw "Endurance failed or ended early: $OutputJson"}
Write-Host "Endurance PASS: $OutputJson"
