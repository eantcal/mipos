#!/usr/bin/env sh
set -eu

build_dir="build-stm8-sdcc"
mcu_model="STM8S208"
sim=0
rebuild=0

do_help() {
    cat <<USAGE
Usage: sh scripts/run-stm8-sdcc.sh [--sim] [--rebuild] [--build-dir DIR] [--mcu MODEL]

Build the STM8 example with SDCC. With --sim, start uCsim's STM8 simulator after the
image is built.
USAGE
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --sim) sim=1 ;;
        --rebuild) rebuild=1 ;;
        --build-dir)
            shift
            build_dir="$1"
            ;;
        --mcu)
            shift
            mcu_model="$1"
            ;;
        -h|--help)
            do_help
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            do_help >&2
            exit 2
            ;;
    esac
    shift
done

need_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "$1 was not found. $2" >&2
        exit 1
    fi
}

need_cmd cmake "Install CMake and make sure it is in PATH."
need_cmd sdcc "Install SDCC with STM8 support, for example: sudo apt install sdcc."

if [ "$rebuild" -eq 1 ] && [ -d "$build_dir" ]; then
    case "$(cd "$build_dir" && pwd)" in
        "$(pwd)"/*) rm -rf "$build_dir" ;;
        *) echo "Refusing to remove unexpected build path: $build_dir" >&2; exit 1 ;;
    esac
fi

cmake -S . -B "$build_dir" \
    -DMIPOS_TARGET_STM8_SDCC=ON \
    -DMIPOS_STM8_MCU_MODEL="$mcu_model"
cmake --build "$build_dir" --target stm8-sdcc-helloworld --parallel

image="$build_dir/stm8-sdcc/stm8s_example.ihx"
echo "STM8 SDCC image: $image"

if [ "$sim" -eq 1 ]; then
    simulator=$(command -v ucsim_stm8 || command -v sstm8 || true)
    if [ -z "$simulator" ]; then
        echo "ucsim_stm8/sstm8 was not found. Install the SDCC uCsim package, for example: sudo apt install sdcc-ucsim." >&2
        exit 1
    fi
    echo "Starting uCsim STM8 simulator. Type 'quit' at the simulator prompt to exit."
    exec "$simulator" -p "sstm8> " "$image"
fi

echo "Build complete. Re-run with --sim to start the STM8 simulator if sdcc-ucsim is installed."