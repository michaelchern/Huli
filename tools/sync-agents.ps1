<#
    sync-agents.ps1
    ----------------
    Discover and check synchronization between human-maintained Chinese
    agent files and English AI-facing files.

    Usage:
        .\tools\sync-agents.ps1
        .\tools\sync-agents.ps1 -Prompt
            Print the prompt to give an AI agent after editing Chinese sources.
            Running the script without switches prints the same prompt.

        .\tools\sync-agents.ps1 -Check
            Check discovered pairs, normalized source SHA256 markers, orphaned
            English files, duplicate markers, and untranslated Chinese text.

    This script does not modify files.
#>
Param(
    [switch]$Check,
    [switch]$Prompt
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$repoRoot = (Resolve-Path "$PSScriptRoot\..").Path

function Get-AbsoluteRepoPath {
    param([string]$RelativePath)
    return Join-Path $repoRoot ($RelativePath -replace "/", "\")
}

function ConvertTo-RepoPath {
    param([string]$AbsolutePath)

    $resolvedPath = (Resolve-Path -LiteralPath $AbsolutePath).Path
    if (-not $resolvedPath.StartsWith($repoRoot, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Path is outside the repository: $AbsolutePath"
    }

    return $resolvedPath.Substring($repoRoot.Length).TrimStart("\", "/").Replace("\", "/")
}

function ConvertTo-MarkerSlug {
    param([string]$Value)

    return (($Value -replace "[^A-Za-z0-9]+", "_").Trim([char[]]"_")).ToUpperInvariant()
}

function Get-NormalizedSha256 {
    param([string]$Path)

    $text = Get-Content -Raw -Encoding UTF8 -LiteralPath $Path
    $normalized = $text.Replace("`r`n", "`n").Replace("`r", "`n")
    $bytes = [Text.UTF8Encoding]::new($false).GetBytes($normalized)
    $sha256 = [Security.Cryptography.SHA256]::Create()
    try {
        $hashBytes = $sha256.ComputeHash($bytes)
    }
    finally {
        $sha256.Dispose()
    }

    return ([BitConverter]::ToString($hashBytes)).Replace("-", "").ToLowerInvariant()
}

function New-SyncPair {
    param(
        [string]$Source,
        [string]$Target,
        [string]$Marker,
        [string]$Label
    )

    return @{
        Source = $Source
        Target = $Target
        Marker = $Marker
        Label = $Label
    }
}

function Get-DocumentPairs {
    param(
        [string]$SourceDirectory,
        [string]$TargetDirectory,
        [string]$MarkerPrefix,
        [string]$LabelPrefix
    )

    $sourceRoot = Get-AbsoluteRepoPath $SourceDirectory
    if (-not (Test-Path -LiteralPath $sourceRoot)) {
        return @()
    }

    return @(
        Get-ChildItem -LiteralPath $sourceRoot -File -Filter "*.md" |
            Sort-Object Name |
            ForEach-Object {
                $source = ConvertTo-RepoPath $_.FullName
                $target = "$TargetDirectory/$($_.Name)"
                $stem = [IO.Path]::GetFileNameWithoutExtension($_.Name)
                $marker = "${MarkerPrefix}_$(ConvertTo-MarkerSlug $stem)_ZH_CN_SHA256"
                New-SyncPair $source $target $marker "$LabelPrefix $stem"
            }
    )
}

function Get-SkillPairs {
    $skillsRoot = Get-AbsoluteRepoPath ".agents/skills"
    if (-not (Test-Path -LiteralPath $skillsRoot)) {
        return @()
    }

    return @(
        Get-ChildItem -LiteralPath $skillsRoot -Directory |
            Sort-Object Name |
            ForEach-Object {
                $sourcePath = Join-Path $_.FullName "SKILL.zh-CN.md"
                if (Test-Path -LiteralPath $sourcePath) {
                    $skillSlug = ConvertTo-MarkerSlug $_.Name
                    New-SyncPair -Source (ConvertTo-RepoPath $sourcePath) -Target (".agents/skills/{0}/SKILL.md" -f $_.Name) -Marker "${skillSlug}_SKILL_ZH_CN_SHA256" -Label "skill $($_.Name)"
                }
            }
    )
}

function Get-OrphanedEnglishDocuments {
    param(
        [string]$TargetDirectory,
        [hashtable]$KnownTargets
    )

    $targetRoot = Get-AbsoluteRepoPath $TargetDirectory
    if (-not (Test-Path -LiteralPath $targetRoot)) {
        return @()
    }

    return @(
        Get-ChildItem -LiteralPath $targetRoot -File -Filter "*.md" |
            ForEach-Object { ConvertTo-RepoPath $_.FullName } |
            Where-Object { -not $KnownTargets.ContainsKey($_) } |
            Sort-Object
    )
}

function Get-OrphanedEnglishSkills {
    param([hashtable]$KnownTargets)

    $skillsRoot = Get-AbsoluteRepoPath ".agents/skills"
    if (-not (Test-Path -LiteralPath $skillsRoot)) {
        return @()
    }

    return @(
        Get-ChildItem -LiteralPath $skillsRoot -Directory |
            ForEach-Object {
                $targetPath = Join-Path $_.FullName "SKILL.md"
                if (Test-Path -LiteralPath $targetPath) {
                    ConvertTo-RepoPath $targetPath
                }
            } |
            Where-Object { -not $KnownTargets.ContainsKey($_) } |
            Sort-Object
    )
}

$syncPairs = @(
    New-SyncPair "AGENTS.zh-CN.md" "AGENTS.md" "AGENTS_ZH_CN_SHA256" "root AGENTS"
)
$syncPairs += Get-DocumentPairs "docs/agents/zh-CN" "docs/agents" "AGENT_DOCS" "agent context"
$syncPairs += Get-DocumentPairs "docs/tasks/zh-CN" "docs/tasks" "TASK_DOCS" "task document"
$syncPairs += Get-SkillPairs

$knownTargets = @{}
foreach ($pair in $syncPairs) {
    $knownTargets[$pair.Target] = $true
}

$orphanedTargets = @()
$orphanedTargets += Get-OrphanedEnglishDocuments "docs/agents" $knownTargets
$orphanedTargets += Get-OrphanedEnglishDocuments "docs/tasks" $knownTargets
$orphanedTargets += Get-OrphanedEnglishSkills $knownTargets

function Get-SyncState {
    param([hashtable]$Pair)

    $sourcePath = Get-AbsoluteRepoPath $Pair.Source
    $targetPath = Get-AbsoluteRepoPath $Pair.Target

    if (-not (Test-Path -LiteralPath $sourcePath)) {
        throw "Missing source file: $($Pair.Source)"
    }

    $sourceHash = Get-NormalizedSha256 $sourcePath
    $targetExists = Test-Path -LiteralPath $targetPath
    $targetText = if ($targetExists) {
        Get-Content -Raw -Encoding UTF8 -LiteralPath $targetPath
    }
    else {
        ""
    }
    $markerPattern = "<!-- $([regex]::Escape($Pair.Marker)): ([a-fA-F0-9]{64}) -->"
    $markerMatches = @([regex]::Matches($targetText, $markerPattern))
    $storedHash = if ($markerMatches.Count -eq 1) {
        $markerMatches[0].Groups[1].Value.ToLowerInvariant()
    }
    else {
        ""
    }
    $containsChinese = $targetText -match "[\u3400-\u4DBF\u4E00-\u9FFF]"

    return [pscustomobject]@{
        Label = $Pair.Label
        Source = $Pair.Source
        Target = $Pair.Target
        Marker = $Pair.Marker
        SourceHash = $sourceHash
        StoredHash = $storedHash
        TargetExists = $targetExists
        MarkerCount = $markerMatches.Count
        ContainsChinese = $containsChinese
        InSync = (
            $targetExists -and
            $markerMatches.Count -eq 1 -and
            $storedHash -eq $sourceHash -and
            -not $containsChinese
        )
    }
}

$states = @($syncPairs | ForEach-Object { Get-SyncState $_ })

if ($Check) {
    $failed = @($states | Where-Object { -not $_.InSync })

    foreach ($state in $failed) {
        if (-not $state.TargetExists) {
            Write-Host "$($state.Target) is missing for $($state.Source)."
            continue
        }
        if ($state.MarkerCount -eq 0) {
            Write-Host "$($state.Target) has no sync marker for $($state.Source)."
        }
        elseif ($state.MarkerCount -gt 1) {
            Write-Host "$($state.Target) has duplicate sync markers for $($state.Source)."
        }
        elseif ($state.StoredHash -ne $state.SourceHash) {
            Write-Host "$($state.Target) has a stale source marker for $($state.Source)."
            Write-Host "Stored hash:  $($state.StoredHash)"
            Write-Host "Current hash: $($state.SourceHash)"
        }
        if ($state.ContainsChinese) {
            Write-Host "$($state.Target) still contains Chinese text; finish the English synchronization."
        }
    }

    foreach ($target in $orphanedTargets) {
        Write-Host "$target has no matching Chinese source file."
    }

    if ($failed.Count -gt 0 -or $orphanedTargets.Count -gt 0) {
        exit 1
    }

    Write-Host "All discovered English agent files have current source markers and pass English-content checks."
    exit 0
}

$markerLines = $states | ForEach-Object {
    "- $($_.Target): <!-- $($_.Marker): $($_.SourceHash) -->"
}

$pairLines = $states | ForEach-Object {
    "- $($_.Source) -> $($_.Target)"
}

$promptLines = @(
    'Please synchronize the English AI-facing files from their Chinese source files.',
    '',
    'This is the `=sa` project command: sync all discovered agent context files.',
    '',
    'Rules:',
    '- Treat Chinese files as the source of truth.',
    '- Create a missing English target and remove or resolve orphaned English documents or skills.',
    '- Keep English files concise, direct, and free of untranslated Chinese text.',
    '- Preserve command names, paths, warnings, validation rules, and forbidden actions.',
    '- Preserve each file''s purpose and section structure where practical.',
    '- Add or update exactly one matching marker in each English file:',
    ($markerLines -join [Environment]::NewLine),
    '- Do not modify Chinese source files unless explicitly asked.',
    '- Do not modify unrelated files.',
    '- Do not create `.agents/skills/*/zh-CN/SKILL.md`; use `SKILL.zh-CN.md` for Chinese skill sources to avoid duplicate skill discovery.',
    '',
    'Pairs:',
    ($pairLines -join [Environment]::NewLine),
    '',
    'After editing, run:',
    '.\tools\sync-agents.ps1 -Check'
)

if ($orphanedTargets.Count -gt 0) {
    $promptLines += ''
    $promptLines += 'Orphaned English files to resolve:'
    $promptLines += ($orphanedTargets | ForEach-Object { "- $_" })
}

Write-Output ($promptLines -join [Environment]::NewLine)
