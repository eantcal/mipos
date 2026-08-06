# Build and Test

CMake is the canonical build entry point for the modern mipOS workspace. The
old checked-in Visual Studio solution and project files were removed; generate
IDE projects on demand from CMake.

## Windows x64 simulator

```powershell
cmake -S . -B build-vs-x64 -G "Visual Studio 18 2026" -A x64
cmake --build build-vs-x64 --config Release --parallel
```

For Visual Studio 2022, use generator `Visual Studio 17 2022` instead.

## Windows Win32 simulator

```powershell
cmake -S . -B build-vs-win32 -G "Visual Studio 18 2026" -A Win32
cmake --build build-vs-win32 --config Release --parallel
```

## Linux or macOS native simulator

```sh
cmake -S . -B build
cmake --build build --parallel
```

For explicit Linux x64 or x86 simulator builds:

```sh
cmake -S . -B build-linux-x64 -DMIPOS_ARCH=x64
cmake --build build-linux-x64 --parallel

cmake -S . -B build-linux-x86 -DMIPOS_ARCH=x86
cmake --build build-linux-x86 --parallel
```

Linux x86 requires multilib compiler and runtime support.

## Tests

Unit tests and smoke tests are built through CMake:

```powershell
cmake --build build-vs-x64 --config Release --target mipos_tests mipos_scheduler_smoke
.\build-vs-x64\Release\mipos_tests.exe
.\build-vs-x64\Release\mipos_scheduler_smoke.exe
```

lwIP tests are enabled in a networking build:

```powershell
cmake --build build-vs-x64-net-check --config Release --target mipos_lwip_tests
.\build-vs-x64-net-check\Release\mipos_lwip_tests.exe
```

The current tests cover queues, memory pools, malloc/realloc, cooperative
scheduler behavior, and optional lwIP IPv4/Ethernet protocol paths.

## QEMU ARM builds

Cortex-M QEMU firmware:

```powershell
.\scripts\run-qemu.ps1 helloworld
.\scripts\run-qemu.ps1 console
.\scripts\run-qemu.ps1 filesystem
.\scripts\run-qemu.ps1 fatfs
# FatFs host-backed block device; details: Storage And FatFs wiki page
.\scripts\run-qemu.ps1 net-console
.\scripts\run-qemu.ps1 net-selftest
```

Cortex-A `vexpress-a9` smoke boot:

```powershell
.\scripts\run-qemu.ps1 vexpress-helloworld
```

Direct CMake build for `vexpress-a9`:

```sh
cmake -S . -B build-qemu-vexpress-a9 \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake \
      -DMIPOS_TARGET_QEMU_VEXPRESS_A9=ON
cmake --build build-qemu-vexpress-a9 --parallel
```


## STM8 SDCC Build

The STM8 port has an optional SDCC build path:

```sh
cmake -S . -B build-stm8-sdcc -DMIPOS_TARGET_STM8_SDCC=ON
cmake --build build-stm8-sdcc --target stm8-sdcc-helloworld --parallel
```

The output image is:

```text
build-stm8-sdcc/stm8-sdcc/stm8s_example.ihx
```

Helper scripts build the image and can start the STM8 simulator when uCsim is installed:

```powershell
.\scripts\run-stm8-sdcc.ps1
.\scripts\run-stm8-sdcc.ps1 -Sim
```

```sh
sh scripts/run-stm8-sdcc.sh
sh scripts/run-stm8-sdcc.sh --sim
```

CI includes an `stm8-sdcc` job that installs SDCC, builds the firmware, and
checks that the Intel HEX image is produced.

## CI expectations

CI should keep simulator builds green across supported hosts and keep QEMU ARM
firmware smoke tests building. Runtime QEMU smoke tests are useful for checking
that firmware reaches the expected console output before QEMU is terminated by
a timeout wrapper.
