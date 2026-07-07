#!/bin/sh
# Launch a mipOS ARM firmware under QEMU (board: lm3s6965evb).
#
# Usage:
#   ./scripts/run-qemu.sh                # filesystem demo (default)
#   ./scripts/run-qemu.sh console        # interactive mipOS console (CLI)
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

usage() {
    cat <<EOF
Usage:
  $0 [filesystem|console|helloworld] [--rebuild] [--gdb]

Firmware:
  filesystem    interactive mipOS tiny filesystem demo (default)
  console       interactive mipOS console
  helloworld    boot demo that prints periodic ticks

Options:
  --rebuild     force CMake configure/build before launching QEMU
  --gdb         start QEMU halted with a GDB server on :1234

Quit QEMU with Ctrl-A x.
EOF
}

for arg in "$@"; do
    case "$arg" in
        filesystem | console | helloworld) firmware="$arg" ;;
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
done

repo_root=$(cd "$(dirname "$0")/.." && pwd)
build_dir="$repo_root/build-qemu-arm"

case "$firmware" in
    filesystem) elf="$build_dir/mipos-arm-qemu-filesystem.elf" ;;
    console) elf="$build_dir/mipos-arm-qemu-console.elf" ;;
    helloworld) elf="$build_dir/mipos-arm-qemu.elf" ;;
esac

command -v qemu-system-arm > /dev/null 2>&1 || {
    echo "qemu-system-arm not found: install QEMU" >&2
    exit 1
}

if [ "$rebuild" = 1 ] || [ ! -f "$elf" ]; then
    command -v arm-none-eabi-gcc > /dev/null 2>&1 || {
        echo "arm-none-eabi-gcc not found: install the Arm GNU toolchain" >&2
        exit 1
    }

    cmake -S "$repo_root" -B "$build_dir" \
        -DCMAKE_TOOLCHAIN_FILE="$repo_root/cmake/arm-none-eabi-toolchain.cmake" \
        -DMIPOS_TARGET_QEMU_ARM=ON
    cmake --build "$build_dir" --parallel
fi

set -- -M lm3s6965evb -kernel "$elf" -nographic -serial mon:stdio
if [ "$gdb" = 1 ]; then
    set -- "$@" -S -s
    echo "GDB server on :1234 (arm-none-eabi-gdb $elf; target remote :1234)"
fi

echo "Starting QEMU ($(basename "$elf")) - quit with Ctrl-A x"
exec qemu-system-arm "$@"
