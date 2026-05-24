$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot

$jmeHeader = Get-Content -Raw (Join-Path $root "include/jme/jme.h")
$jmeExtendedHeader = Get-Content -Raw (Join-Path $root "include/jme/jme_extended.h")

$jmeText = $jmeHeader + "`n" + $jmeExtendedHeader

$jmeFunctions = [regex]::Matches(
    $jmeText,
    "(?m)^\s*(?:const\s+char\s*\*\s*|char\s*\*\s*|void\s+|double\s+|int\s+)(jme_[a-zA-Z0-9_]+)\s*\("
) | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique

$jmeConstants = [regex]::Matches($jmeText, "\bJME_[A-Z0-9_]+\b") |
    ForEach-Object { $_.Value } |
    Sort-Object -Unique

if ($jmeFunctions.Count -le 0) {
    throw "Expected defined JME function surface, found $($jmeFunctions.Count)"
}

if ($jmeFunctions.Count -ne 204) {
    throw "Expected JME function surface = 204, found $($jmeFunctions.Count)"
}

if ($jmeConstants.Count -ne 462) {
    throw "Expected JME constant surface = 462, found $($jmeConstants.Count)"
}

Write-Output "jme_functions=$($jmeFunctions.Count)"
Write-Output "jme_constants=$($jmeConstants.Count)"
