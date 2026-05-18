$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot

$jmeHeader = Get-Content -Raw (Join-Path $root "include/jme/jme.h")
$jmeExtendedHeader = Get-Content -Raw (Join-Path $root "include/jme/jme_extended.h")
$referenceHeader = Get-Content -Raw (Join-Path $root "include/jme_compat/swephexp.h")

$jmeText = $jmeHeader + "`n" + $jmeExtendedHeader

$jmeFunctions = [regex]::Matches(
    $jmeText,
    "(?m)^\s*(?:const\s+char\s*\*|void|double|int)\s+(jme_[a-zA-Z0-9_]+)\s*\("
) | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique

$jmeConstants = [regex]::Matches($jmeText, "\bJME_[A-Z0-9_]+\b") |
    ForEach-Object { $_.Value } |
    Sort-Object -Unique

$referenceConstants = [regex]::Matches(
    $referenceHeader,
    "(?m)^#define\s+(SE[A-Z0-9_]*|SEMOD[A-Z0-9_]*|OK|ERR)\b"
) | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique

if ($referenceConstants.Count -ne 348) {
    throw "Expected 348 reference constants, found $($referenceConstants.Count)"
}

if ($jmeFunctions.Count -le 0) {
    throw "Expected defined JME function surface, found $($jmeFunctions.Count)"
}

if ($jmeConstants.Count -lt 348) {
    throw "Expected JME constant surface >= 348, found $($jmeConstants.Count)"
}

Write-Output "reference_constants=$($referenceConstants.Count)"
Write-Output "jme_functions=$($jmeFunctions.Count)"
Write-Output "jme_constants=$($jmeConstants.Count)"
