# STM8 SDCC

The STM8 port can be built with the Small Device C Compiler as an optional
cross target. This keeps the historical STM8 BSP useful without requiring the
older proprietary Cosmic or Raisonance toolchains.

## Status

The SDCC path now builds and executes two firmware images:

- `stm8-sdcc-helloworld` produces `stm8s_example.ihx`;
- `stm8-sdcc-console` produces `stm8s_console.ihx`;
- the console runs the standard mipOS commands over the STM8S208 UART model;
- the SDCC task context saves the return PC, stack pointer, and `Y` register;
- uCsim runs at 16 MHz and TIM4 advances the kernel RTC through a scheduler
  polling hook.

The CMake build uses explicit custom commands instead of treating SDCC as a
hosted C compiler. That keeps the freestanding invocation predictable and
makes the generated Intel HEX images easy to use from WSL and CI.

## Requirements

Install SDCC with STM8 support. On Debian or Ubuntu this is typically:

```sh
sudo apt install sdcc
```

To launch the optional CPU simulator, also install uCsim's STM8 simulator:

```sh
sudo apt install sdcc-ucsim
```

On Windows, install SDCC from the project packages and make sure `sdcc.exe` is
in `PATH`. If the installer includes `ucsim_stm8.exe` or `sstm8.exe`, the run script can start the
simulator too.

Useful upstream references:

- [SDCC project](https://sdcc.sourceforge.net/)
- [ucsim_stm8 manual page](https://manpages.debian.org/unstable/sdcc-ucsim/ucsim_stm8.1.en.html)

## Build With CMake

```sh
cmake -S . -B build-stm8-sdcc -DMIPOS_TARGET_STM8_SDCC=ON
cmake --build build-stm8-sdcc --target stm8-sdcc-helloworld --parallel
```

The build creates an Intel HEX image:

```text
build-stm8-sdcc/stm8-sdcc/stm8s_example.ihx
```

To select another STM8S/A device define:

```sh
cmake -S . -B build-stm8-sdcc \
      -DMIPOS_TARGET_STM8_SDCC=ON \
      -DMIPOS_STM8_MCU_MODEL=STM8S103
```

## Helper Scripts

PowerShell:

```powershell
.\scripts\run-stm8-sdcc.ps1
.\scripts\run-stm8-sdcc.ps1 -Sim
.\scripts\run-stm8-sdcc.ps1 -Rebuild -McuModel STM8S103
```

Shell:

```sh
sh scripts/run-stm8-sdcc.sh
sh scripts/run-stm8-sdcc.sh --example console --sim
sh scripts/run-stm8-sdcc.sh --test-console --rebuild
sh scripts/run-stm8-sdcc.sh --rebuild --mcu STM8S103
```

The scripts stop early with an actionable message if SDCC is missing. With the
simulation option they also require `ucsim_stm8`/`sstm8`; otherwise they only build the HEX
image.

## Legacy Makefile

`examples/stm8/Makefile` remains available for a small direct SDCC build:

```sh
cd examples/stm8
make
make sim
make clean
```

It now writes object files and the HEX image under `examples/stm8/build/`
instead of placing generated `.rel` files next to project sources.

## Simulation Scope

For the console image, uCsim models the STM8S208 CPU and UART. mipOS UART data
is available on `localhost:7777`; the uCsim command monitor listens on
`localhost:7778`.

Run the interactive console:

```sh
sh scripts/run-stm8-sdcc.sh --example console --sim
nc localhost 7777
```

Run the automated smoke test:

```sh
sh scripts/run-stm8-sdcc.sh --test-console --rebuild
```

The smoke test uses UART flow control at character granularity because the
STM8 UART has no receive FIFO. It waits for each echoed character and for the
prompt between commands, then checks the output of `help`, `ps`, and
`dump 0 16`. TIM4 remains active in uCsim; its overflow flag is polled by the
scheduler to advance the mipOS RTC without enabling a simulated interrupt
vector.

This is a CPU/peripheral simulation rather than a complete electrical model of
an STM8S Discovery board. GPIO wiring, external devices, and board-level timing
still require hardware or a richer board simulator.

## CI Coverage

GitHub Actions installs `sdcc` and `sdcc-ucsim`, builds the helloworld image,
and runs the console smoke test in uCsim. The job therefore covers image
generation, cooperative context resume, the TIM4 clock path, UART input/output,
and the built-in `help`, `ps`, and `dump` commands.

## Related Pages

- [Supported Targets](Supported-Targets)
- [Build and Test](Build-and-Test)
- [Porting Guide](Porting-Guide)