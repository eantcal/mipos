#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

adapter=""
list=0
ensure_tap=0
enable_nat=0
tap_name="mipos-tap"
host_ip="10.77.0.1"
ip="10.77.0.2"
netmask="255.255.255.0"
gateway="10.77.0.1"
mac="02:00:00:00:00:01"
build_dir="build-linux-x64-net"
rebuild=0

usage() {
    cat <<EOF
Usage:
  $0 --list
  $0 --ensure-tap [--enable-nat]
  $0 --adapter <index-or-name>
  $0 --adapter <index-or-name> --ip 10.77.0.2 --gateway 10.77.0.1

Options:
  --tap-name <name>      TAP interface name, default: $tap_name
  --host-ip <ip>         Linux side TAP address, default: $host_ip
  --ip <ip>              mipOS address, default: $ip
  --netmask <mask>       subnet mask, default: $netmask
  --gateway <ip>         mipOS gateway, default: $gateway
  --mac <mac>            mipOS MAC, default: $mac
  --build-dir <dir>      CMake build directory, default: $build_dir
  --rebuild              rerun CMake configure
EOF
}

prefix_from_netmask() {
    local mask="$1"
    local prefix=0
    local seen_zero=0
    local IFS=.
    local octet bit set

    read -r -a parts <<< "$mask"
    if [ "${#parts[@]}" -ne 4 ]; then
        echo "invalid netmask: $mask" >&2
        exit 2
    fi

    for octet in "${parts[@]}"; do
        if ! [[ "$octet" =~ ^[0-9]+$ ]] || [ "$octet" -lt 0 ] || [ "$octet" -gt 255 ]; then
            echo "invalid netmask: $mask" >&2
            exit 2
        fi

        for bit in 7 6 5 4 3 2 1 0; do
            set=$(( (octet >> bit) & 1 ))
            if [ "$set" -eq 1 ]; then
                if [ "$seen_zero" -eq 1 ]; then
                    echo "non-contiguous netmask: $mask" >&2
                    exit 2
                fi
                prefix=$((prefix + 1))
            else
                seen_zero=1
            fi
        done
    done

    echo "$prefix"
}

network_from_ip_netmask() {
    local address="$1"
    local mask="$2"
    local IFS=.
    local -a address_parts mask_parts network_parts

    read -r -a address_parts <<< "$address"
    read -r -a mask_parts <<< "$mask"
    if [ "${#address_parts[@]}" -ne 4 ] || [ "${#mask_parts[@]}" -ne 4 ]; then
        echo "invalid address/netmask: $address/$mask" >&2
        exit 2
    fi

    for i in 0 1 2 3; do
        network_parts[$i]=$(( address_parts[$i] & mask_parts[$i] ))
    done

    echo "${network_parts[0]}.${network_parts[1]}.${network_parts[2]}.${network_parts[3]}"
}

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "missing required command: $1" >&2
        echo "On Debian/Ubuntu install dependencies with:" >&2
        echo "  sudo apt install cmake build-essential libpcap-dev iproute2" >&2
        exit 3
    fi
}

ensure_tap_interface() {
    local prefix="$1"

    require_command ip

    if ! ip link show "$tap_name" >/dev/null 2>&1; then
        echo "Creating TAP interface '$tap_name'..."
        sudo ip tuntap add dev "$tap_name" mode tap user "$(id -un)"
    fi

    echo "Configuring Linux side $tap_name = $host_ip/$prefix..."
    sudo ip addr flush dev "$tap_name"
    sudo ip addr add "$host_ip/$prefix" dev "$tap_name"
    sudo ip link set "$tap_name" up
}

enable_linux_nat() {
    local prefix="$1"
    local default_dev
    local subnet

    subnet="$(ip route get 1.1.1.1 | awk '{for (i=1; i<=NF; ++i) if ($i=="dev") {print $(i+1); exit}}')"
    default_dev="$subnet"
    if [ -z "$default_dev" ]; then
        echo "Could not detect default route interface for NAT" >&2
        exit 4
    fi

    echo "Enabling IPv4 forwarding..."
    sudo sysctl -w net.ipv4.ip_forward=1 >/dev/null

    if command -v iptables >/dev/null 2>&1; then
        subnet="$(network_from_ip_netmask "$host_ip" "$netmask")/$prefix"
        echo "Adding iptables MASQUERADE rule for $subnet via $default_dev..."
        if ! sudo iptables -t nat -C POSTROUTING -s "$subnet" -o "$default_dev" -j MASQUERADE 2>/dev/null; then
            sudo iptables -t nat -A POSTROUTING -s "$subnet" -o "$default_dev" -j MASQUERADE
        fi
    else
        echo "iptables not found; forwarding is enabled, but NAT was not configured." >&2
        echo "Install iptables or add an equivalent nftables masquerade rule." >&2
    fi
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --help|-h) usage; exit 0 ;;
        --list) list=1; shift ;;
        --ensure-tap) ensure_tap=1; shift ;;
        --enable-nat) enable_nat=1; shift ;;
        --adapter) adapter="$2"; shift 2 ;;
        --tap-name) tap_name="$2"; shift 2 ;;
        --host-ip) host_ip="$2"; shift 2 ;;
        --ip) ip="$2"; shift 2 ;;
        --netmask) netmask="$2"; shift 2 ;;
        --gateway) gateway="$2"; shift 2 ;;
        --mac) mac="$2"; shift 2 ;;
        --build-dir) build_dir="$2"; shift 2 ;;
        --rebuild) rebuild=1; shift ;;
        *) echo "unknown option: $1" >&2; usage; exit 2 ;;
    esac
done

require_command cmake

build_path="$repo_root/$build_dir"
if [ "$rebuild" -eq 1 ] || [ ! -f "$build_path/CMakeCache.txt" ]; then
    cmake -S "$repo_root" -B "$build_path" \
        -DMIPOS_ARCH=x64 \
        -DMIPOS_NET_STACK=lwip \
        -DMIPOS_NET_BACKEND=all
fi

targets=(example-net-pcap)
if [ "$ensure_tap" -eq 1 ]; then
    targets+=(example-net-tap)
fi
cmake --build "$build_path" --target "${targets[@]}"

pcap_exe="$build_path/example-net-pcap"
tap_exe="$build_path/example-net-tap"

if [ "$list" -eq 1 ]; then
    "$pcap_exe" --list
    exit $?
fi

if [ "$ensure_tap" -eq 1 ]; then
    prefix="$(prefix_from_netmask "$netmask")"
    ensure_tap_interface "$prefix"
    gateway="$host_ip"

    if [ "$enable_nat" -eq 1 ]; then
        enable_linux_nat "$prefix"
    fi

    adapter="$tap_name"
    echo "Using TAP adapter '$tap_name'"
    echo "Linux side: $host_ip"
    echo "mipOS side: $ip"
    echo "From another terminal run: ping $ip"
    if [ "$enable_nat" -eq 1 ]; then
        echo "From mipOS-net try: ping 1.1.1.1"
    fi
    echo ""
fi

if [ -z "$adapter" ]; then
    echo "missing --adapter. Run with --list first, or use --ensure-tap." >&2
    exit 2
fi

runner=()
if [ "$(id -u)" -ne 0 ]; then
    runner=(sudo)
fi

if [ "$ensure_tap" -eq 1 ]; then
    "${runner[@]}" "$tap_exe" \
        --name "$adapter" \
        --ip "$ip" \
        --netmask "$netmask" \
        --gateway "$gateway" \
        --mac "$mac"
else
    "${runner[@]}" "$pcap_exe" \
        --adapter "$adapter" \
        --ip "$ip" \
        --netmask "$netmask" \
        --gateway "$gateway" \
        --mac "$mac"
fi
