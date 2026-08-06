param(
    [switch]$Sim,
    [switch]$Rebuild,
    [string]$BuildDir = "build-stm8-sdcc",
    [string]$Generator = "",
    [string]$McuModel = "STM8S208"
)

$ErrorActionPreference = "Stop"

function Require-Command($Name, $Remedy) {
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if (-not $cmd) {
        throw "$Name was not found. $Remedy"
    }
    return $cmd.Source
}

Require-Command cmake "Install CMake and make sure it is in PATH." | Out-Null
Require-Command sdcc "Install SDCC with STM8 support from https://sdcc.sourceforge.net/ and make sure sdcc is in PATH." | Out-Null

if ($Rebuild -and (Test-Path $BuildDir)) {
    $resolved = Resolve-Path $BuildDir
    $root = Resolve-Path (Get-Location)
    if (-not $resolved.Path.StartsWith($root.Path, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove unexpected build path $($resolved.Path)"
    }
    Remove-Item -LiteralPath $resolved.Path -Recurse -Force
}

$configureArgs = @(
    "-S", ".",
    "-B", $BuildDir,
    "-DMIPOS_TARGET_STM8_SDCC=ON",
    "-DMIPOS_STM8_MCU_MODEL=$McuModel"
)

if ($Generator) {
    $configureArgs += @("-G", $Generator)
}

& cmake @configureArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& cmake --build $BuildDir --target stm8-sdcc-helloworld --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$image = Join-Path $BuildDir "stm8-sdcc\stm8s_example.ihx"
Write-Host "STM8 SDCC image: $image"

if ($Sim) {
    $simulator = Get-Command ucsim_stm8 -ErrorAction SilentlyContinue
    if (-not $simulator) { $simulator = Get-Command sstm8 -ErrorAction SilentlyContinue }
    if (-not $simulator) {
        throw "ucsim_stm8/sstm8 was not found. Install the SDCC uCsim package, often named sdcc-ucsim, then retry with -Sim."
    }

    Write-Host "Starting uCsim STM8 simulator. Type 'quit' at the simulator prompt to exit."
    & $simulator.Source -p "sstm8> " $image
    exit $LASTEXITCODE
}

Write-Host "Build complete. Re-run with -Sim to start the STM8 simulator if sdcc-ucsim is installed."