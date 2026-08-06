#!/usr/bin/env bash
set -euo pipefail
export LC_ALL=C

uart_port="${1:-7777}"
prompt="mipOS-stm8>"
transcript=""

read_until() {
    local needle="$1"
    local tail=""
    local ch=""

    while IFS= read -r -N1 -t 30 -u 3 ch; do
        printf '%s' "$ch"
        transcript="${transcript}${ch}"
        tail="${tail}${ch}"

        if [[ "$tail" == *"$needle" ]]; then
            return 0
        fi

        if (( ${#tail} > 128 )); then
            tail="${tail: -64}"
        fi
    done

    echo "Timed out waiting for '$needle' from the STM8 UART." >&2
    return 1
}

send_command() {
    local command="$1"
    local i ch

    for ((i = 0; i < ${#command}; ++i)); do
        ch="${command:i:1}"
        printf '%s' "$ch" >&3
        read_until "$ch"
    done

    printf '\r' >&3
    read_until "$prompt"
}

connected=0
for _ in {1..100}; do
    if { exec 3<>"/dev/tcp/127.0.0.1/${uart_port}"; } 2>/dev/null; then
        connected=1
        break
    fi
    sleep 0.1
done

if (( ! connected )); then
    echo "Could not connect to the STM8 UART on localhost:${uart_port}." >&2
    exit 1
fi

# Synchronize even when the firmware printed its initial prompt before the
# TCP client connected.
send_command ""
send_command "help"
send_command "ps"
send_command "dump 0 16"

for expected in \
    "help - shows this list" \
    "root@" \
    "cnsltx@" \
    "cnslrx@" \
    "0x00000000"; do
    if [[ "$transcript" != *"$expected"* ]]; then
        echo "STM8 console response did not contain: $expected" >&2
        exit 1
    fi
done

echo
echo "STM8 console test passed."
