# Launch a mipOS ARM firmware under QEMU (board: lm3s6965evb).
#
# Usage:
#   .\scripts\run-qemu.ps1                 # filesystem demo (default)
#   .\scripts\run-qemu.ps1 console         # interactive mipOS console (CLI)
#   .\scripts\run-qemu.ps1 fatfs           # FatFs over host-backed disk image
#   .\scripts\run-qemu.ps1 helloworld      # tick demo
#   .\scripts\run-qemu.ps1 -Rebuild        # force a rebuild first
#   .\scripts\run-qemu.ps1 -Gdb            # start halted with GDB server :1234
#   .\scripts\run-qemu.ps1 -Help           # show usage
#
# Quit QEMU with Ctrl-A x.

[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [ValidateSet('filesystem', 'console', 'helloworld', 'fatfs')]
    [string]$Firmware = 'filesystem',

    [string]$DiskImage,
    [switch]$ResetDisk,
    [switch]$Rebuild,
    [switch]$Gdb,
    [switch]$Help
)

$ErrorActionPreference = 'Stop'

if ($Help) {
    @'
Usage:
  .\scripts\run-qemu.ps1 [filesystem|console|helloworld|fatfs] [-DiskImage <path>] [-ResetDisk] [-Rebuild] [-Gdb]

Firmware:
  filesystem    interactive mipOS tiny filesystem demo (default)
  console       interactive mipOS console
  helloworld    boot demo that prints periodic ticks
  fatfs         FatFs demo backed by build-qemu-arm\qemu-fatfs.img

Options:
  -DiskImage    source FAT image copied to qemu-fatfs.img when it is missing
  -ResetDisk    overwrite qemu-fatfs.img from -DiskImage before launch
  -Rebuild      force CMake configure/build before launching QEMU
  -Gdb          start QEMU halted with a GDB server on :1234

Quit QEMU with Ctrl-A x.
'@ | Write-Host
    exit 0
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build-qemu-arm'

$elfName = switch ($Firmware) {
    'filesystem' { 'mipos-arm-qemu-filesystem.elf' }
    'console' { 'mipos-arm-qemu-console.elf' }
    'fatfs' { 'mipos-arm-qemu-fatfs.elf' }
    default { 'mipos-arm-qemu.elf' }
}
$elf = Join-Path $buildDir $elfName
$qemuFatImage = Join-Path $buildDir 'qemu-fatfs.img'

# --- locate qemu-system-arm ---------------------------------------------
$qemu = Get-Command qemu-system-arm -ErrorAction SilentlyContinue
if ($qemu) {
    $qemu = $qemu.Source
} elseif (Test-Path 'C:\Program Files\qemu\qemu-system-arm.exe') {
    $qemu = 'C:\Program Files\qemu\qemu-system-arm.exe'
} else {
    throw 'qemu-system-arm not found: install QEMU (winget install SoftwareFreedomConservancy.QEMU)'
}

# --- build the firmware if needed ----------------------------------------
if ($Rebuild -or -not (Test-Path $elf)) {
    # Make sure arm-none-eabi-gcc is reachable
    if (-not (Get-Command arm-none-eabi-gcc -ErrorAction SilentlyContinue)) {
        $tc = Get-ChildItem 'C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\*\bin' `
                -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($tc) {
            $env:PATH = "$($tc.FullName);$env:PATH"
        } else {
            throw 'arm-none-eabi-gcc not found: install the Arm GNU toolchain (winget install Arm.GnuArmEmbeddedToolchain)'
        }
    }

    Write-Host "Building $elfName..." -ForegroundColor Cyan
    cmake -S $repoRoot -B $buildDir -G Ninja `
        -DCMAKE_TOOLCHAIN_FILE="$repoRoot\cmake\arm-none-eabi-toolchain.cmake" `
        -DMIPOS_TARGET_QEMU_ARM=ON
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed' }

    cmake --build $buildDir
    if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
}

if ($Firmware -eq 'fatfs') {
    if (-not $DiskImage) {
        $DiskImage = Join-Path $repoRoot 'examples\simu\fatfs\disk.img'
    }

    if (-not (Test-Path $DiskImage)) {
        throw "FAT disk image not found: $DiskImage"
    }

    if ($ResetDisk -or -not (Test-Path $qemuFatImage)) {
        New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
        Copy-Item -LiteralPath $DiskImage -Destination $qemuFatImage -Force
    }
}

# --- run ------------------------------------------------------------------
$qemuArgs = @('-M', 'lm3s6965evb', '-kernel', $elf, '-nographic',
              '-serial', 'mon:stdio')
if ($Firmware -eq 'fatfs') {
    $qemuArgs += @('-semihosting-config', 'enable=on,target=native')
}
if ($Gdb) {
    $qemuArgs += @('-S', '-s')
    Write-Host 'GDB server on :1234 (arm-none-eabi-gdb ' -NoNewline
    Write-Host "$elf" -NoNewline
    Write-Host '; target remote :1234)' -ForegroundColor Yellow
}

Write-Host "Starting QEMU ($elfName) - quit with Ctrl-A x" -ForegroundColor Green
Push-Location $(if ($Firmware -eq 'fatfs') { $buildDir } else { $repoRoot })
try {
    & $qemu @qemuArgs
} finally {
    Pop-Location
}
