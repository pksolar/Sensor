param(
    [string]$Compiler = 'D:\software\keil\ARM\ARMCLANG\bin\armclang.exe'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path

& (Join-Path $root 'tests\host\run_tests.ps1')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& (Join-Path $root 'tools\refactor\check_contracts.ps1')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& (Join-Path $root 'tools\refactor\check_armclang_syntax.ps1') -Compiler $Compiler
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Push-Location $root
try {
    git diff --check
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
finally {
    Pop-Location
}

Write-Output 'PASS: software-only validation suite completed'
