$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot

function Read-Text([string] $path) {
    return Get-Content -Raw (Join-Path $root $path)
}

$jmeHeader = Read-Text "include/jme/jme.h"
$jmeExtendedHeader = Read-Text "include/jme/jme_extended.h"
$sourceText = (
    Get-ChildItem -Path (Join-Path $root "src") -Filter "*.c" |
        ForEach-Object { Get-Content -Raw $_.FullName }
) -join "`n"
$testText = (
    Get-ChildItem -Path (Join-Path $root "tests") -Filter "*.c" |
        ForEach-Object { Get-Content -Raw $_.FullName }
) -join "`n"
$projectText = (
    Get-ChildItem -Path $root -Recurse -File |
        Where-Object {
            $_.FullName -notmatch '[/\\]build[/\\]' -and
            $_.FullName -notmatch '[/\\]\.git[/\\]' -and
            $_.Extension -in @(".c", ".h", ".md", ".ps1", ".txt")
        } |
        ForEach-Object { Get-Content -Raw $_.FullName }
) -join "`n"

$jmeText = $jmeHeader + "`n" + $jmeExtendedHeader

$jmeFunctions = [regex]::Matches(
    $jmeText,
    "(?m)^\s*(?:const\s+char\s*\*\s*|char\s*\*\s*|void\s+|double\s+|int\s+)(jme_[a-zA-Z0-9_]+)\s*\("
) | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique

$jmeConstants = [regex]::Matches($jmeText, "\bJME_[A-Z0-9_]+\b") |
    ForEach-Object { $_.Value } |
    Sort-Object -Unique

$missingJme = New-Object System.Collections.Generic.List[string]
$untestedJme = New-Object System.Collections.Generic.List[string]

foreach ($fn in $jmeFunctions) {
    $hasDefinition = $sourceText -match "(?m)^\s*(?:const\s+char\s*\*\s*|char\s*\*\s*|void\s+|double\s+|int\s+)$([regex]::Escape($fn))\s*\("

    if (!$hasDefinition) {
        $missingJme.Add($fn)
    }

    $hasDirectTestReference = $testText -match "\b$([regex]::Escape($fn))\s*\("
    if (!$hasDirectTestReference) {
        $untestedJme.Add($fn)
    }
}

if ($missingJme.Count -gt 0) {
    throw "Declared JME functions without definitions: $($missingJme -join ', ')"
}

if ($untestedJme.Count -gt 0) {
    throw "Declared JME functions without direct C test references: $($untestedJme -join ', ')"
}

$forbiddenBytes = @(
    @(115, 116, 117, 98),
    @(112, 108, 97, 99, 101, 104, 111, 108, 100, 101, 114),
    @(109, 111, 99, 107),
    @(102, 97, 107, 101),
    @(115, 97, 109, 112, 108, 101),
    @(100, 101, 109, 111),
    @(102, 105, 120, 109, 101),
    @(104, 97, 99, 107),
    @(116, 111, 100, 111),
    @(110, 111, 116, 32, 105, 109, 112, 108, 101, 109, 101, 110, 116, 101, 100),
    @(117, 110, 105, 109, 112, 108, 101, 109, 101, 110, 116, 101, 100)
)

foreach ($bytes in $forbiddenBytes) {
    $word = -join ($bytes | ForEach-Object { [char]$_ })
    if ($projectText -match "(?i)\b$([regex]::Escape($word))\b") {
        throw "Forbidden marker found: $word"
    }
}

if ($jmeConstants.Count -lt 348) {
    throw "JME constants below target: $($jmeConstants.Count)"
}

Write-Output "jme_functions_total=$($jmeFunctions.Count)"
Write-Output "jme_functions_defined=$($jmeFunctions.Count)"
Write-Output "jme_functions_directly_tested=$($jmeFunctions.Count)"
Write-Output "jme_constants_total=$($jmeConstants.Count)"
