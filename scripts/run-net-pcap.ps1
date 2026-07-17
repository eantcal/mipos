# Build and run the mipOS lwIP Windows network demos.
#
# Usage:
#   .\scripts\run-net-pcap.ps1 -List
#   .\scripts\run-net-pcap.ps1 -EnsureTap
#   .\scripts\run-net-pcap.ps1 -EnsureTap -EnableNat
#   .\scripts\run-net-pcap.ps1 -Adapter 3
#   .\scripts\run-net-pcap.ps1 -Adapter 3 -Ip 10.77.0.2 -Gateway 10.77.0.1

[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$Adapter,

    [switch]$List,
    [switch]$EnsureTap,
    [switch]$EnableNat,
    [string]$TapName = 'mipOS TAP',
    [string]$NatName = 'mipOSNat',
    [string]$HostIp = '10.77.0.1',
    [string]$Ip = '10.77.0.2',
    [string]$Netmask = '255.255.255.0',
    [string]$Gateway = '10.77.0.1',
    [string]$Mac = '02:00:00:00:00:01',
    [string]$BuildDir = 'build-vs-x64-net',
    [switch]$Rebuild,
    [string]$Generator = 'Visual Studio 18 2026'
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildPath = Join-Path $repoRoot $BuildDir

function Test-IsAdministrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Invoke-SelfElevated {
    $args = @(
        '-NoLogo',
        '-NoProfile',
        '-ExecutionPolicy',
        'Bypass',
        '-File',
        $PSCommandPath
    )

    foreach ($entry in $PSBoundParameters.GetEnumerator()) {
        if ($entry.Value -is [switch]) {
            if ($entry.Value.IsPresent) {
                $args += "-$($entry.Key)"
            }
        } elseif ($null -ne $entry.Value) {
            $args += "-$($entry.Key)"
            $args += [string]$entry.Value
        }
    }

    Write-Host ""
    Write-Host "TAP setup requires Administrator privileges. Requesting elevation..." -ForegroundColor Yellow
    Write-Host ""

    Start-Process -FilePath 'powershell.exe' `
        -ArgumentList $args `
        -WorkingDirectory $repoRoot `
        -Verb RunAs
    exit 5
}

function Show-TapInstallHelp {
    param([string]$Name)

    Write-Host ""
    Write-Host "TAP adapter '$Name' was not found and tapctl.exe is not installed." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Install OpenVPN TAP-Windows, then rerun this script from an Administrator PowerShell:" -ForegroundColor Yellow
    Write-Host "  winget install OpenVPNTechnologies.OpenVPN"
    Write-Host ""
    Write-Host "Alternative with Chocolatey:"
    Write-Host "  choco install openvpn -y"
    Write-Host ""
    Write-Host "After installation:"
    Write-Host "  cd $repoRoot"
    Write-Host "  .\scripts\run-net-pcap.ps1 -EnsureTap"
    Write-Host ""
    Write-Host "tapctl.exe was searched in:"
    Write-Host "  PATH"
    Write-Host "  $env:ProgramFiles\OpenVPN\bin\tapctl.exe"
    Write-Host "  ${env:ProgramFiles(x86)}\OpenVPN\bin\tapctl.exe"
    Write-Host ""
}

function Find-TapCtl {
    $cmd = Get-Command tapctl.exe -ErrorAction SilentlyContinue
    if ($cmd) {
        return $cmd.Source
    }

    $candidates = @(
        "$env:ProgramFiles\OpenVPN\bin\tapctl.exe",
        "${env:ProgramFiles(x86)}\OpenVPN\bin\tapctl.exe",
        "$env:ProgramFiles\OpenVPN Connect\tapctl.exe",
        "${env:ProgramFiles(x86)}\OpenVPN Connect\tapctl.exe"
    )

    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path $candidate)) {
            return $candidate
        }
    }

    return $null
}

function Get-TapAdapter {
    param([string]$Name)

    Get-NetAdapter -ErrorAction SilentlyContinue |
        Where-Object {
            $_.Name -eq $Name -or
            $_.InterfaceDescription -like '*TAP-Windows*' -or
            $_.InterfaceDescription -like '*TAP Adapter*'
        } |
        Select-Object -First 1
}

function Get-NpcapNameForAdapter {
    param($AdapterObject)

    if (-not $AdapterObject.InterfaceGuid) {
        throw "Adapter '$($AdapterObject.Name)' has no InterfaceGuid"
    }

    return "\Device\NPF_{$($AdapterObject.InterfaceGuid.ToString().ToUpperInvariant())}"
}

function ConvertTo-PrefixLength {
    param([string]$Mask)

    $prefixLength = 0
    $seenZero = $false

    foreach ($part in $Mask.Split('.')) {
        $value = [int]$part
        if ($value -lt 0 -or $value -gt 255) {
            throw "Invalid netmask '$Mask'"
        }

        for ($bit = 7; $bit -ge 0; --$bit) {
            $set = ($value -band (1 -shl $bit)) -ne 0
            if ($set) {
                if ($seenZero) {
                    throw "Netmask '$Mask' is not contiguous"
                }
                ++$prefixLength
            } else {
                $seenZero = $true
            }
        }
    }

    return $prefixLength
}

function Get-NetworkPrefix {
    param(
        [string]$Address,
        [string]$Mask
    )

    $addressParts = $Address.Split('.')
    $maskParts = $Mask.Split('.')

    if ($addressParts.Count -ne 4 -or $maskParts.Count -ne 4) {
        throw "Invalid address/netmask '$Address/$Mask'"
    }

    $networkParts = @()
    for ($i = 0; $i -lt 4; ++$i) {
        $networkParts += ([int]$addressParts[$i] -band [int]$maskParts[$i])
    }

    $prefixLength = ConvertTo-PrefixLength -Mask $Mask
    return "$($networkParts -join '.')/$prefixLength"
}

function Ensure-WindowsNat {
    param(
        [string]$Name,
        [string]$InterfaceAlias,
        [string]$InternalPrefix
    )

    if (-not (Test-IsAdministrator)) {
        Write-Host ""
        Write-Host "Windows NAT setup requires an Administrator PowerShell." -ForegroundColor Yellow
        Write-Host ""
        exit 4
    }

    Write-Host "Enabling IPv4 forwarding on '$InterfaceAlias'..." -ForegroundColor Cyan
    Set-NetIPInterface -InterfaceAlias $InterfaceAlias `
        -AddressFamily IPv4 `
        -Forwarding Enabled | Out-Null

    $nat = Get-NetNat -Name $Name -ErrorAction SilentlyContinue
    if ($nat -and $nat.InternalIPInterfaceAddressPrefix -ne $InternalPrefix) {
        Write-Host "Recreating NAT '$Name' for prefix $InternalPrefix..." -ForegroundColor Cyan
        Remove-NetNat -Name $Name -Confirm:$false
        $nat = $null
    }

    if (-not $nat) {
        Write-Host "Creating Windows NAT '$Name' for $InternalPrefix..." -ForegroundColor Cyan
        New-NetNat -Name $Name -InternalIPInterfaceAddressPrefix $InternalPrefix | Out-Null
    } else {
        Write-Host "Windows NAT '$Name' already covers $InternalPrefix" -ForegroundColor Green
    }
}

function Ensure-TapAdapter {
    param(
        [string]$Name,
        [string]$Address,
        [string]$Mask
    )

    $adapterObject = Get-TapAdapter -Name $Name

    if (-not $adapterObject) {
        $tapctl = Find-TapCtl
        if (-not $tapctl) {
            Show-TapInstallHelp -Name $Name
            exit 3
        }

        if (-not (Test-IsAdministrator)) {
            Write-Host ""
            Write-Host "Creating TAP adapter '$Name' requires an Administrator PowerShell." -ForegroundColor Yellow
            Write-Host "Rerun:"
            Write-Host "  cd $repoRoot"
            Write-Host "  .\scripts\run-net-pcap.ps1 -EnsureTap"
            Write-Host ""
            exit 4
        }

        Write-Host "Creating TAP adapter '$Name'..." -ForegroundColor Cyan
        & $tapctl create --name $Name
        if ($LASTEXITCODE -ne 0) {
            & $tapctl create --name $Name --hwid tap0901
        }
        if ($LASTEXITCODE -ne 0) {
            throw 'tapctl failed to create the TAP adapter. Run this shell as Administrator.'
        }

        Start-Sleep -Seconds 2
        $adapterObject = Get-TapAdapter -Name $Name
    }

    if (-not $adapterObject) {
        throw "TAP adapter '$Name' could not be found after creation"
    }

    if ($adapterObject.Name -ne $Name) {
        if (-not (Test-IsAdministrator)) {
            Write-Host ""
            Write-Host "Renaming TAP adapter '$($adapterObject.Name)' to '$Name' requires an Administrator PowerShell." -ForegroundColor Yellow
            Write-Host ""
            exit 4
        }

        Write-Host "Renaming TAP adapter '$($adapterObject.Name)' to '$Name'..." -ForegroundColor Cyan
        Rename-NetAdapter -Name $adapterObject.Name -NewName $Name
        Start-Sleep -Seconds 1
        $adapterObject = Get-NetAdapter -Name $Name
    }

    if (-not (Test-IsAdministrator)) {
        Write-Host ""
        Write-Host "Configuring IPv4 address on '$Name' requires an Administrator PowerShell." -ForegroundColor Yellow
        Write-Host "Rerun:"
        Write-Host "  cd $repoRoot"
        Write-Host "  .\scripts\run-net-pcap.ps1 -EnsureTap"
        Write-Host ""
        exit 4
    }

    Write-Host "Configuring Windows side $Name = $Address/$Mask..." -ForegroundColor Cyan
    netsh interface ip set address name="$Name" static $Address $Mask | Out-Host
    if ($LASTEXITCODE -ne 0) {
        throw 'Failed to configure TAP IPv4 address. Run this shell as Administrator.'
    }

    Enable-NetAdapter -Name $Name -Confirm:$false -ErrorAction SilentlyContinue | Out-Null

    $adapterObject = Get-NetAdapter -Name $Name
    return $adapterObject
}

if (($EnsureTap -or $EnableNat) -and -not (Test-IsAdministrator)) {
    Invoke-SelfElevated
}

if ($Rebuild -or -not (Test-Path (Join-Path $buildPath 'mipos-examples.slnx'))) {
    cmake -S $repoRoot -B $buildPath -G $Generator -A x64 `
        -DMIPOS_ARCH=x64 `
        -DMIPOS_NET_STACK=lwip `
        -DMIPOS_NET_BACKEND=all

    if ($LASTEXITCODE -ne 0) {
        throw 'CMake configuration failed'
    }
}

$targets = if ($EnsureTap) {
    @('example-net-tap')
} else {
    @('example-net-pcap', 'example-net-tap')
}

cmake --build $buildPath --config Release --target $targets
if ($LASTEXITCODE -ne 0) {
    throw 'network examples build failed'
}

$pcapExe = Join-Path $buildPath 'Release\example-net-pcap.exe'
$tapExe = Join-Path $buildPath 'Release\example-net-tap.exe'

if ($List) {
    & $pcapExe --list
    exit $LASTEXITCODE
}

if ($EnsureTap) {
    $tap = Ensure-TapAdapter -Name $TapName -Address $HostIp -Mask $Netmask
    $Gateway = $HostIp
    $natPrefix = Get-NetworkPrefix -Address $HostIp -Mask $Netmask

    if ($EnableNat) {
        Ensure-WindowsNat -Name $NatName `
            -InterfaceAlias $tap.Name `
            -InternalPrefix $natPrefix
    }

    Write-Host "Using TAP adapter '$($tap.Name)' directly" -ForegroundColor Green
    Write-Host "Windows side: $HostIp"
    Write-Host "mipOS side:   $Ip"
    Write-Host "From another Windows terminal run: ping $Ip"
    if ($EnableNat) {
        Write-Host "NAT enabled: $natPrefix -> Windows default route"
        Write-Host "From mipOS-net try: route 1.1.1.1"
        Write-Host "From mipOS-net try: ping 1.1.1.1"
    }
    Write-Host ""

    & $tapExe `
        --guid $tap.InterfaceGuid `
        --ip $Ip `
        --netmask $Netmask `
        --gateway $Gateway `
        --mac $Mac
    exit $LASTEXITCODE
}

if (-not $Adapter) {
    throw 'Missing -Adapter. Run with -List first, or use -EnsureTap.'
}

& $pcapExe --adapter $Adapter --ip $Ip --netmask $Netmask --gateway $Gateway --mac $Mac
exit $LASTEXITCODE
