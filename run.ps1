param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Args
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $PSCommandPath
Set-Location $scriptDir

function Invoke-Interpreter {
    param(
        [string[]]$InterpreterArgs
    )

    $cmakeExe = Join-Path $scriptDir "build/interpreter.exe"
    if (Test-Path $cmakeExe) {
        & $cmakeExe @InterpreterArgs
        exit $LASTEXITCODE
    }

    $fallbackExe = Join-Path $scriptDir "interpreter_test.exe"
    if (-not (Test-Path $fallbackExe)) {
        g++ src/*.cpp -I include -o interpreter_test.exe
    }

    & $fallbackExe @InterpreterArgs
    exit $LASTEXITCODE
}

$cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
$nmakeCmd = Get-Command nmake -ErrorAction SilentlyContinue
if ($null -ne $cmakeCmd -and $null -ne $nmakeCmd) {
    try {
        $toolchain = $null
        if ($env:VCPKG_ROOT) {
            $candidate = Join-Path $env:VCPKG_ROOT "scripts/buildsystems/vcpkg.cmake"
            if (Test-Path $candidate) {
                $toolchain = $candidate
            }
        }

        if ($toolchain) {
            cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$toolchain"
        } else {
            cmake -B build -S .
        }

        cmake --build build
        Invoke-Interpreter -InterpreterArgs $Args
    }
    catch {
        Invoke-Interpreter -InterpreterArgs $Args
    }
}

Invoke-Interpreter -InterpreterArgs $Args
