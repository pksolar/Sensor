param(
    [string]$Compiler = 'D:\software\keil\ARM\ARMCLANG\bin\armclang.exe'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path

if (-not (Test-Path -LiteralPath $Compiler)) {
    throw "Arm Compiler 6 syntax checker not found: $Compiler"
}

[xml]$project = Get-Content -LiteralPath (Join-Path $root '7500HA-YY.uvprojx')
$sources = @(
    $project.SelectNodes('//FilePath') |
        ForEach-Object { $_.InnerText } |
        Where-Object { $_.EndsWith('.c') }
)
$includes = @(
    '-IHK32F0xxA_StdPeriphDriver/inc',
    '-ICMSIS/HK32F0xxA/Include',
    '-Iuser/inc',
    '-ICMSIS/CM0/Core',
    '-IMyDriver/inc',
    '-ICoeffCountMod'
)
$common = @(
    '--target=arm-arm-none-eabi',
    '-mcpu=cortex-m0',
    '-std=c99',
    '-fsyntax-only',
    '-Wno-armcc-pragma-import',
    '-Wno-parentheses-equality',
    '-Wno-empty-body',
    '-Wno-format',
    '-DUSE_STDPERIPH_DRIVER',
    '-DHK32F030xxA'
)

$failed = @()
Push-Location $root
try {
    foreach ($rawPath in $sources) {
        $source = ($rawPath -replace '^\.\\', '') -replace '\\', '/'
        & $Compiler @common @includes $source
        if ($LASTEXITCODE -ne 0) {
            $failed += $source
        }
    }
}
finally {
    Pop-Location
}

if ($failed.Count -ne 0) {
    throw "Arm Compiler 6 syntax failures: $($failed -join ', ')"
}

Write-Output "PASS: $($sources.Count) Keil C sources accepted by Arm Compiler 6 syntax check"
