#!/usr/bin/env sh
set -eu

build_dir="build-stm8-sdcc"
mcu_model="STM8S208"
example="helloworld"
sim=0
rebuild=0
test_console=0
uart_port="7777"
console_port="7778"

do_help() {
    cat <<USAGE
Usage: sh scripts/run-stm8-sdcc.sh [--sim] [--rebuild] [--build-dir DIR] [--mcu MODEL]
                                 [--example helloworld|console]
                                 [--uart-port PORT] [--console-port PORT]
                                 [--test-console]

Build an STM8 example with SDCC. With --sim, start uCsim after the image is
built. The console example exposes the simulated UART on localhost:PORT.
With --test-console, it verifies help, ps, and dump over that UART.
USAGE
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --sim) sim=1 ;;
        --test-console) test_console=1 ;;
        --rebuild) rebuild=1 ;;
        --build-dir)
            shift
            build_dir="$1"
            ;;
        --mcu)
            shift
            mcu_model="$1"
            ;;
        --example)
            shift
            example="$1"
            ;;
        --uart-port)
            shift
            uart_port="$1"
            ;;
        --console-port)
            shift
            console_port="$1"
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

if [ "$test_console" -eq 1 ]; then
    example="console"
    sim=1
fi

case "$example" in
    helloworld)
        target="stm8-sdcc-helloworld"
        image_name="stm8s_example.ihx"
        ;;
    console)
        target="stm8-sdcc-console"
        image_name="stm8s_console.ihx"
        ;;
    *)
        echo "Unsupported example: $example (use helloworld or console)" >&2
        exit 2
        ;;
esac

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
cmake --build "$build_dir" --target "$target" --parallel

image="$build_dir/stm8-sdcc/$image_name"
echo "STM8 SDCC image: $image"

if [ "$sim" -eq 1 ]; then
    simulator=$(command -v ucsim_stm8 || command -v sstm8 || true)
    if [ -z "$simulator" ]; then
        echo "ucsim_stm8/sstm8 was not found. Install the SDCC uCsim package, for example: sudo apt install sdcc-ucsim." >&2
        exit 1
    fi

    if [ "$example" = "console" ]; then
        if [ "$test_console" -eq 1 ]; then
            sim_log=$(mktemp)
            "$simulator" -q -g -X 16M \
                -Z "$console_port" \
                -S "uart=1,port=$uart_port,raw" \
                -e "set error stack off" \
                -e "pc 0x8010" \
                "$image" >"$sim_log" 2>&1 &
            sim_pid=$!

            cleanup_simulator() {
                kill "$sim_pid" 2>/dev/null || true
                wait "$sim_pid" 2>/dev/null || true
            }
            trap cleanup_simulator EXIT INT TERM

            if ! bash scripts/test-stm8-console.sh "$uart_port"; then
                cleanup_simulator
                trap - EXIT INT TERM
                echo "uCsim output:" >&2
                sed -n '1,160p' "$sim_log" >&2
                exit 1
            fi

            cleanup_simulator
            trap - EXIT INT TERM
            rm -f "$sim_log"
            exit 0
        fi

        echo "Starting uCsim STM8 console firmware."
        echo "mipOS UART: localhost:$uart_port"
        echo "uCsim command console: localhost:$console_port"
        echo "From another WSL terminal run: nc localhost $uart_port"
        exec "$simulator" -g -X 16M -Z "$console_port" \
            -S "uart=1,port=$uart_port,raw" \
            -e "set error stack off" \
            -e "pc 0x8010" \
            "$image"
    fi

    echo "Starting uCsim STM8 simulator. Type 'run' at the simulator prompt to execute, 'quit' to exit."
    exec "$simulator" -p "sstm8> " "$image"
fi

echo "Build complete. Re-run with --sim to start the STM8 simulator if sdcc-ucsim is installed."
