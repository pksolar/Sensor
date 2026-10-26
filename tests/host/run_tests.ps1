$ErrorActionPreference = 'Stop'

$pythonCandidates = @(
    'C:\Users\pk\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe',
    'python.exe',
    'python'
)

$python = $null
foreach ($candidate in $pythonCandidates) {
    if (Test-Path -LiteralPath $candidate) {
        $python = $candidate
        break
    }
    $command = Get-Command $candidate -ErrorAction SilentlyContinue
    if ($command) {
        $python = $command.Source
        break
    }
}

if (-not $python) {
    throw 'Python runtime not found; host characterization tests were not run.'
}

& $python "$PSScriptRoot\test_characterization.py"
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
