# mipOS

[![CI](https://github.com/eantcal/mipos/actions/workflows/ci.yml/badge.svg)](https://github.com/eantcal/mipos/actions/workflows/ci.yml)

![mipOS Logo](mipos_pictures/mipos.png)

Multum In Parvo Operating System is a cooperative multitasking RTOS designed
for single-chip microcontrollers with severely limited RAM and ROM. It provides
tasks, mutual exclusion, signals, message queues, memory management, malloc,
memory pools, timers, and optional console, file-system, and stdio support.

mipOS is written mainly in C and is intended to stay compact, portable, and easy
to study. It can also run as a user-space simulator on Windows, Linux, and macOS.

![mipOS Architecture](mipos_pictures/miposarc.png)

The original design and source code were published in an article for the
magazine Computer Programming, describing the internals of a portable and
scalable kernel for resource-constrained microcontrollers. A modern, in-depth
description of the kernel internals is available in
[docs/mipos-internals.md](docs/mipos-internals.md).

mipOS has been tested on Intel x86/x64 simulator targets, STM8, STM32, and
ARM Cortex-M3 microcontroller families. The simulator also builds on ARM64
hosts; on macOS ARM64, the C/C++ unit tests run but the cooperative scheduler
smoke test is disabled because the simulator context-switch backend is not
currently supported on Apple Silicon.

![mipOS running in an evaluation board](mipos_pictures/mipos2.jpg)

See also: [https://www.eantcal.eu/home/mipos](https://www.eantcal.eu/home/mipos).
For release notes and recent project changes, see [NEWS.md](NEWS.md).

![mipOS IDE screenshot](mipos_pictures/miposIDE.jpg)

## Building

CMake is the supported entry point for desktop simulator builds. The old
checked-in Visual Studio solution and project files have been removed; generate
Visual Studio solutions on demand from CMake instead.

### Windows, Visual Studio x64

```powershell
cmake -S . -B build-vs-x64 -G "Visual Studio 18 2026" -A x64
cmake --build build-vs-x64 --config Release --parallel
```

For Visual Studio 2022, use:

```powershell
cmake -S . -B build-vs-x64 -G "Visual Studio 17 2022" -A x64
```

### Windows, Visual Studio x86

```powershell
cmake -S . -B build-vs-win32 -G "Visual Studio 18 2026" -A Win32
cmake --build build-vs-win32 --config Release --parallel
```

### Linux or macOS, native architecture

```sh
cmake -S . -B build
cmake --build build --parallel
```

The native build supports x86-64 and AArch64 hosts. On Apple Silicon, CMake
builds the simulator natively and runs the non-scheduler unit tests; the
cooperative scheduler smoke test is skipped because the simulator context
switching backend is not supported on macOS ARM64.

### Linux x64

```sh
cmake -S . -B build-linux-x64 -DMIPOS_ARCH=x64
cmake --build build-linux-x64 --parallel
```

### Linux x86

```sh
cmake -S . -B build-linux-x86 -DMIPOS_ARCH=x86
cmake --build build-linux-x86 --parallel
```

Linux x86 builds require a 32-bit multilib toolchain and runtime. On Debian or
Ubuntu, install the equivalent of `gcc-multilib` and `g++-multilib`.

### macOS x64

```sh
cmake -S . -B build-macos-x64 -DMIPOS_ARCH=x64
cmake --build build-macos-x64 --parallel
```

`MIPOS_ARCH=x64` sets `CMAKE_OSX_ARCHITECTURES=x86_64` (on Apple Silicon the
binaries run under Rosetta; prefer the native build there). `MIPOS_ARCH=x86`
maps to `i386`, which requires a legacy macOS SDK/toolchain with 32-bit
executable support.

## Tests

GoogleTest-based unit tests are available through CMake. They are enabled by
default when `BUILD_TESTING` is on, or explicitly with `MIPOS_BUILD_TESTS=ON`.

```powershell
cmake -S . -B build-vs-x64 -G "Visual Studio 18 2026" -A x64 -DMIPOS_BUILD_TESTS=ON
cmake --build build-vs-x64 --config Release --target mipos_tests --parallel
ctest --test-dir build-vs-x64 -C Release --output-on-failure
```

The current test targets cover:

- queue FIFO and capacity behavior
- fixed-size memory pool allocation and reuse
- mipOS malloc/free/realloc basics
- cooperative multitasking: task creation, round-robin yielding, and task
  termination through the scheduler
- optional lwIP Ethernet/IPv4 integration: ARP, ICMP, UDP, TCP, DHCP, and DNS
  over a simulated link driver

The test suite also runs in CI (GitHub Actions) on Linux x64/x86, Windows
x64/Win32, and macOS ARM64. The macOS ARM64 job builds the project and runs
the GoogleTest unit tests; `mipos_scheduler_smoke` is intentionally not
registered on that host architecture.

## Current status

- CMake is the canonical build system for desktop simulator builds and for the
  QEMU ARM port.
- Checked-in Visual Studio solutions/projects were removed; generate them with
  CMake when needed.
- GoogleTest covers queues, memory pools, and malloc behavior.
- `mipos_scheduler_smoke` exercises cooperative scheduling on supported
  simulator hosts: Linux x64/x86 and Windows x64/Win32.
- Optional networking can be enabled with `-DMIPOS_NET_STACK=lwip`. The current
  lwIP port is `NO_SYS=1`, IPv4/Ethernet only, and enables ARP, ICMP, UDP,
  TCP raw APIs, DHCP client, and DNS client. IPv6, sockets, and netconn are
  not enabled.
- macOS ARM64 support is currently build/unit-test coverage only for the
  simulator. Scheduler context switching on Apple Silicon needs a dedicated
  backend before scheduler examples or smoke tests can be considered supported.

## Optional networking

The default build does not include a TCP/IP stack. To enable the experimental
lwIP port, configure with `MIPOS_NET_STACK=lwip`:

```powershell
cmake -S . -B build-vs-x64-lwip -G "Visual Studio 18 2026" -A x64 -DMIPOS_NET_STACK=lwip
cmake --build build-vs-x64-lwip --config Release --target mipos_lwip_tests
.\build-vs-x64-lwip\Release\mipos_lwip_tests.exe
```

CMake downloads lwIP 2.2.1 from the official Savannah release archive and
builds only the subset needed by mipOS. The first driver model is an Ethernet
`netif` adapter with receive injection and link-output callbacks; the test
coverage verifies ARP, ICMP echo reply, UDP output, TCP connect/listen setup,
DHCP discover output, and DNS query output without requiring hardware.

### Windows TAP/Npcap ping test

For an end-to-end host test, use Npcap to capture and inject Ethernet frames
on a Layer-2 adapter such as TAP-Windows. This keeps ARP visible to mipOS, so
the first real service test can be the normal Windows `ping` command.

1. Install Npcap and a TAP adapter, for example the OpenVPN TAP-Windows driver.
2. In an Administrator PowerShell, create/configure the TAP adapter and start
   the mipOS side:

   ```powershell
   .\scripts\run-net-pcap.ps1 -EnsureTap
   ```

   This looks for an existing `mipOS TAP` adapter, otherwise tries to create
   one with OpenVPN `tapctl.exe`, configures the Windows side as `10.77.0.1`,
   and starts mipOS as `10.77.0.2`.

   To let mipOS reach hosts beyond the TAP subnet, also enable Windows NAT:

   ```powershell
   .\scripts\run-net-pcap.ps1 -EnsureTap -EnableNat
   ```

   This enables IPv4 forwarding on the TAP interface and creates a Windows
   `NetNat` named `mipOSNat` for the `10.77.0.0/24` internal prefix.

3. To inspect adapters manually instead:

   ```powershell
   .\scripts\run-net-pcap.ps1 -List
   ```

4. Or start the mipOS lwIP bridge on a specific TAP/Npcap adapter index:

   ```powershell
   .\scripts\run-net-pcap.ps1 -Adapter 3 -Ip 10.77.0.2 -Gateway 10.77.0.1
   ```

5. In another terminal, test ARP + ICMP:

   ```powershell
   ping 10.77.0.2
   ```

By default the TAP console keeps packet logs disabled. Type `verbose` to show
compact packet logs such as `RX ARP who-has 10.77.0.2`,
`TX ARP is-at 10.77.0.2`, `RX ICMP echo request`, and `TX ICMP echo reply`;
type `quiet` to hide them again. `example-net-pcap.exe` still supports
`--quiet` when run directly.

The TAP backend also exposes a small interactive console on the mipOS side:

```text
mipOS-net> help
mipOS-net> route
mipOS-net> route 1.1.1.1
mipOS-net> arp 10.77.0.1
mipOS-net> ping 10.77.0.1
mipOS-net> ping 1.1.1.1
mipOS-net> udp 10.77.0.1 9000 hello
mipOS-net> tcp 1.1.1.1 80
mipOS-net> dhcp status
mipOS-net> dns server 1.1.1.1
mipOS-net> dns example.com
mipOS-net> stats
```

The console keeps an in-memory command history for the current session; use
the Up/Down arrow keys to recall previous commands.

`route` prints the current IPv4 routing view:

```text
mipOS-net> route
destination      gateway          netmask          iface  type
10.77.0.0        0.0.0.0          255.255.255.0    mip0   connected
0.0.0.0          10.77.0.1        0.0.0.0          mip0   default

mipOS-net> route 1.1.1.1
destination      next-hop         iface  type
1.1.1.1          10.77.0.1        mip0   default
```

`arp` should resolve the Windows-side TAP address. `ping 10.77.0.1` sends an
ICMP echo request from mipOS to Windows; if the packet log shows the outgoing
request but no reply, allow inbound ICMP echo requests for that Windows network
profile/firewall rule and try again. With `-EnableNat`, `ping 1.1.1.1` should
route via `10.77.0.1`; ICMP through Windows NAT can still depend on local
firewall/NAT policy, so the packet log is the first thing to inspect.

The `udp`, `tcp`, `dhcp`, and `dns` commands exercise lwIP raw APIs from the
TAP console. `udp` emits one datagram, `tcp` starts a single connect attempt,
`dhcp start|stop|status` controls the DHCP client, and `dns server <ip>` plus
`dns <name>` configures and tests the DNS client. The host TAP setup does not
provide a DHCP server by itself; use static addressing unless a DHCP service is
running on that Layer-2 segment.

The `example-net-pcap` binary loads `wpcap.dll` dynamically, so the project
does not need the Npcap SDK to compile. Wintun is a useful future backend for
Layer-3 IP packet tests, but it does not expose Ethernet/ARP frames, so TAP is
the better first target for ARP and ping.

### QEMU ARP/ICMP self-test

The QEMU port can also build a cross-compiled lwIP self-test firmware. It uses
the same `netif` adapter with an in-memory link, so it validates ARP and ICMP
inside the ARM/QEMU environment while the real Stellaris Ethernet driver is
still pending:

```powershell
cmake -S . -B build-qemu-arm -G Ninja `
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake `
      -DMIPOS_TARGET_QEMU_ARM=ON `
      -DMIPOS_QEMU_ARM_NET_SELFTEST=ON
cmake --build build-qemu-arm --target mipos-arm-qemu-net-selftest
.\scripts\run-qemu.ps1 net-selftest
```

## Examples

Simulator examples are built by default. Disable them with
`-DMIPOS_BUILD_EXAMPLES=OFF`.

Built example binaries use the `example-` prefix, for example:

```sh
./build/example-filesystem
```

On Visual Studio generators, binaries are emitted under the configuration
directory, for example:

```powershell
.\build-vs-x64\Release\example-filesystem.exe
```

The filesystem example loads mipOS inside a host process and mounts a disk image
containing sample files.

## Bare-metal ARM port (QEMU)

Besides the host simulator, mipOS can be built as native bare-metal ARM
Cortex-M firmware and executed by QEMU. The port lives in
`mipos/arch/qemu-arm/` and targets the QEMU `lm3s6965evb` machine
(TI Stellaris LM3S6965, Cortex-M3): UART0 console, SysTick-driven 1 ms RTC
quantum, cooperative context switch built on newlib setjmp/longjmp.

Requirements: the [Arm GNU toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
(`arm-none-eabi-gcc`) and `qemu-system-arm`.

### Build

```sh
cmake -S . -B build-qemu-arm \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake \
      -DMIPOS_TARGET_QEMU_ARM=ON
cmake --build build-qemu-arm --parallel
```

This produces four firmwares (each with `.elf`, `.bin`, and `.map`):

- `mipos-arm-qemu.elf` - hello-world demo: boots mipOS and prints a tick
  every second
- `mipos-arm-qemu-console.elf` - interactive mipOS console (CLI) on the
  emulated UART
- `mipos-arm-qemu-filesystem.elf` - interactive console with a RAM-backed
  mipOS tiny filesystem; try `ls` and `cat test1`
- `mipos-arm-qemu-fatfs.elf` - interactive console with FatFs over a
  host-backed block device through QEMU semihosting

### Run

The launch scripts build the firmware on first use and start QEMU:

```powershell
.\scripts\run-qemu.ps1                # filesystem demo
.\scripts\run-qemu.ps1 console        # mipOS console (CLI)
.\scripts\run-qemu.ps1 fatfs          # FatFs host-backed block device
.\scripts\run-qemu.ps1 net-selftest   # lwIP ARP/ICMP self-test
.\scripts\run-qemu.ps1 helloworld     # tick demo
```

Generated Visual Studio solutions also include utility targets under
`QEMU Examples`: `qemu-filesystem`, `qemu-console`, `qemu-fatfs`, and
`qemu-helloworld`. Build one of those targets to launch the corresponding
firmware through `scripts/run-qemu.ps1`.

From `cmd.exe` on Windows, use the wrapper:

```bat
scripts\run-qemu.cmd                  rem filesystem demo
scripts\run-qemu.cmd console          rem mipOS console (CLI)
scripts\run-qemu.cmd fatfs            rem FatFs host-backed block device
scripts\run-qemu.cmd net-selftest     rem lwIP ARP/ICMP self-test
scripts\run-qemu.cmd helloworld       rem tick demo
```

```sh
./scripts/run-qemu.sh                 # filesystem demo
./scripts/run-qemu.sh console         # mipOS console (CLI)
./scripts/run-qemu.sh fatfs           # FatFs host-backed block device
./scripts/run-qemu.sh net-selftest    # lwIP ARP/ICMP self-test
./scripts/run-qemu.sh helloworld      # tick demo
```

or run QEMU directly:

```sh
qemu-system-arm -M lm3s6965evb \
    -kernel build-qemu-arm/mipos-arm-qemu.elf \
    -nographic -serial mon:stdio
```

The hello-world firmware boots mipOS, starts a root task, and prints on the
emulated UART:

```text
mipOS ARM/QEMU port booting...
root task running
tick: 1
tick: 2
...
```

The filesystem firmware is the default launch-script demo. It presents the
`mipOS>` prompt and preloads two files in RAM:

```text
mipOS tiny filesystem demo
Type 'ls' and 'cat test1'.

mipOS>ls
Volume label RAMDISK
test1       56 bytes
test2       55 bytes

mipOS>cat test1
test1       56 bytes
0123456789abcdefghijklmnopqrstuvwxyz0123ABCDEFGHIJKLMNO
```

The console firmware presents the same prompt without the filesystem commands;
type `help` for the list of built-in commands (`ver`, `ps`, `dump`, `freeze`,
`signal`, ...):

```text
mipOS>ps
pid: 01     root@0x119 psig:00000000 wsig:00000001 status: 8  suspend ...
pid: 02   cnsltx@0x1579 psig:00000000 wsig:00000002 status: 8  suspend ...
pid: 03   cnslrx@0x1411 psig:00000000 wsig:00000000 status: 4  running ...
```

Exit QEMU with `Ctrl-A x`.

### Debug

Start QEMU halted with a GDB server on port 1234 (or use the launch scripts
with `-Gdb` / `--gdb`):

```sh
qemu-system-arm -M lm3s6965evb \
    -kernel build-qemu-arm/mipos-arm-qemu.elf \
    -nographic -S -s
```

Then, in another terminal:

```sh
arm-none-eabi-gdb build-qemu-arm/mipos-arm-qemu.elf
(gdb) target remote :1234
(gdb) break Reset_Handler
(gdb) continue
```

### Current limitations

- UART RX is polled (`mipos_bsp_rs232_recv_byte` is non-blocking); no UART
  interrupts are used.
- The SysTick ISR only accumulates ticks; the kernel RTC is updated from the
  scheduler epoch hook in thread context, so the RTC advances only while the
  scheduler is running (which is always the case in a cooperative system).
- Kernel, tasks, and ISRs all share the main stack (MSP); there is no
  PSP/privilege separation.

```text
mipOS v.1.99 - (c) 2005-2017 antonino.calderone@gmail.com console
Compile date/time: Dec 27 2020 00:33:50 - opt. mods:
  mipos_console
  mipos_fs
  mipos_stdio
  mipos_mm
  mipos_malloc
  mipos_mpool

mipOS>help
    help - shows this list
    ver - shows mipOS version
    dump [<a>,[<c>,[<b>]] - dump a memory area
    patch <a> <b> - set a byte <b> at address <a>
    ps - shows the list of tasks
    freeze <tid> - freezes a task
    unfreeze <tid> - unfreezes a task
    delete <tid> - delete a task
    signal <tid> <signum> - set a signal
    ls - shows list of files
    cat - shows list and content of files

mipOS>ls
Volume label RAMDISK
test1       60 bytes
test2       60 bytes
```

## License

mipOS is provided in source form and licensed under the MIT License. See
[COPYING](COPYING) for details.
