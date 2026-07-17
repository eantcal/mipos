#!/bin/sh
# Launch a mipOS ARM firmware under QEMU (board: lm3s6965evb).
#
# Usage:
#   ./scripts/run-qemu.sh                # filesystem demo (default)
#   ./scripts/run-qemu.sh console        # interactive mipOS console (CLI)
#   ./scripts/run-qemu.sh fatfs          # FatFs over host-backed disk image
#   ./scripts/run-qemu.sh net-selftest   # lwIP ARP/ICMP self-test
#   ./scripts/run-qemu.sh helloworld     # tick demo
#   ./scripts/run-qemu.sh --rebuild      # force a rebuild first
#   ./scripts/run-qemu.sh --gdb          # start halted with GDB server :1234
#   ./scripts/run-qemu.sh --help         # show usage
#
# Quit QEMU with Ctrl-A x.

set -e

firmware=filesystem
rebuild=0
gdb=0
disk_image=
reset_disk=0

usage() {
    cat <<EOF
Usage:
  $0 [filesystem|console|helloworld|fatfs|net-selftest] [--disk-image <path>] [--reset-disk] [--rebuild] [--gdb]

Firmware:
  filesystem    interactive mipOS tiny filesystem demo (default)
  console       interactive mipOS console
  helloworld    boot demo that prints periodic ticks
  fatfs         FatFs demo backed by build-qemu-arm/qemu-fatfs.img
  net-selftest  lwIP ARP/ICMP self-test with an in-memory link

Options:
  --disk-image  source FAT image copied to qemu-fatfs.img when it is missing
  --reset-disk  overwrite qemu-fatfs.img from --disk-image before launch
  --rebuild     force CMake configure/build before launching QEMU
  --gdb         start QEMU halted with a GDB server on :1234

Quit QEMU with Ctrl-A x.
EOF
}

while [ "$#" -gt 0 ]; do
    arg="$1"
    case "$arg" in
        filesystem | console | helloworld | fatfs | net-selftest) firmware="$arg" ;;
        --disk-image)
            shift
            disk_image="${1:-}"
            [ -n "$disk_image" ] || {
                usage >&2
                exit 2
            }
            ;;
        --reset-disk) reset_disk=1 ;;
        --rebuild) rebuild=1 ;;
        --gdb) gdb=1 ;;
        --help | -h)
            usage
            exit 0
            ;;
        *)
            usage >&2
            exit 2
            ;;
    esac
    shift
done

repo_root=$(cd "$(dirname "$0")/.." && pwd)
build_dir="$repo_root/build-qemu-arm"

case "$firmware" in
    filesystem) elf="$build_dir/mipos-arm-qemu-filesystem.elf" ;;
    console) elf="$build_dir/mipos-arm-qemu-console.elf" ;;
    fatfs) elf="$build_dir/mipos-arm-qemu-fatfs.elf" ;;
    net-selftest) elf="$build_dir/mipos-arm-qemu-net-selftest.elf" ;;
    helloworld) elf="$build_dir/mipos-arm-qemu.elf" ;;
esac
qemu_fat_image="$build_dir/qemu-fatfs.img"

command -v qemu-system-arm > /dev/null 2>&1 || {
    echo "qemu-system-arm not found: install QEMU" >&2
    exit 1
}

if [ "$rebuild" = 1 ] || [ ! -f "$elf" ]; then
    command -v arm-none-eabi-gcc > /dev/null 2>&1 || {
        echo "arm-none-eabi-gcc not found: install the Arm GNU toolchain" >&2
        exit 1
    }

    cmake_args="-DMIPOS_TARGET_QEMU_ARM=ON"
    if [ "$firmware" = net-selftest ]; then
        cmake_args="$cmake_args -DMIPOS_QEMU_ARM_NET_SELFTEST=ON"
    fi

    cmake -S "$repo_root" -B "$build_dir" \
        -DCMAKE_TOOLCHAIN_FILE="$repo_root/cmake/arm-none-eabi-toolchain.cmake" \
        $cmake_args
    cmake --build "$build_dir" --parallel
fi

if [ "$firmware" = fatfs ]; then
    if [ -z "$disk_image" ]; then
        disk_image="$repo_root/examples/simu/fatfs/disk.img"
    fi

    if [ ! -f "$disk_image" ]; then
        echo "FAT disk image not found: $disk_image" >&2
        exit 1
    fi

    mkdir -p "$build_dir"
    if [ "$reset_disk" = 1 ] || [ ! -f "$qemu_fat_image" ]; then
        cp "$disk_image" "$qemu_fat_image"
    fi
fi

set -- -M lm3s6965evb -kernel "$elf" -nographic -serial mon:stdio
if [ "$firmware" = fatfs ]; then
    set -- "$@" -semihosting-config enable=on,target=native
fi
if [ "$gdb" = 1 ]; then
    set -- "$@" -S -s
    echo "GDB server on :1234 (arm-none-eabi-gdb $elf; target remote :1234)"
fi

echo "Starting QEMU ($(basename "$elf")) - quit with Ctrl-A x"
if [ "$firmware" = fatfs ]; then
    cd "$build_dir"
else
    cd "$repo_root"
fi
exec qemu-system-arm "$@"
