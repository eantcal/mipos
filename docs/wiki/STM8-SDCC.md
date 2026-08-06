# STM8 SDCC

The STM8 port can be built with the Small Device C Compiler as an optional
cross target. This keeps the historical STM8 BSP useful without requiring the
older proprietary Cosmic or Raisonance toolchains.

## Status

The current SDCC path is a first buildable baseline:

- target selector: `MIPOS_TARGET_STM8_SDCC=ON`;
- compiler: `sdcc -mstm8`;
- default device define: `STM8S208`;
- output image: `build-stm8-sdcc/stm8-sdcc/stm8s_example.ihx`;
- example application: `examples/stm8/main.c`;
- BSP: `mipos/arch/stm8/mipos_bsp_stm8s.c`;
- timer/reset/watchdog support through the local STM8S Standard Peripheral
  Library subset.

The CMake build uses explicit custom commands instead of treating SDCC as a
hosted C compiler. That keeps the invocation predictable and avoids CMake
probing assumptions that are not useful for a freestanding 8-bit target.

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
sh scripts/run-stm8-sdcc.sh --sim
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

`ucsim_stm8`/`sstm8` simulates the STM8 CPU and can load the generated Intel HEX image. This
is useful for early toolchain and instruction-level bring-up. It is not a full
board model with the exact UART/timer/watchdog wiring of an STM8S Discovery
board, so the first automated coverage is intentionally a compile-and-image
smoke test.

The next useful simulation step is a tiny SDCC-specific smoke firmware that
exercises startup, a cooperative yield path, and a serial byte path against
uCsim streams. After that, the STM8 BSP can grow toward a repeatable console
smoke test.

## CI Coverage

GitHub Actions includes an `stm8-sdcc` job that installs `sdcc` and
`sdcc-ucsim`, configures `MIPOS_TARGET_STM8_SDCC=ON`, builds
`stm8-sdcc-helloworld`, and checks that the `.ihx` image is produced.

## Related Pages

- [Supported Targets](Supported-Targets)
- [Build and Test](Build-and-Test)
- [Porting Guide](Porting-Guide)