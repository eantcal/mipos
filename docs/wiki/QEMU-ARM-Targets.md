# QEMU ARM Targets

mipOS keeps three QEMU ARM profiles side by side. They are not interchangeable:
each one answers a different engineering question.

| Profile | Machine | CPU style | Role |
|---|---|---|---|
| Workhorse | `lm3s6965evb` | Cortex-M3, Thumb | current bare-metal firmware demos and tests |
| Didactic board | `vexpress-a9` | Cortex-A9, ARM mode | step-by-step ARMv7-A board bring-up |
| Future CI target | `virt` | virtual ARM platform | planned virtio block/network automation target |

## Requirements

QEMU firmware builds require:

- `qemu-system-arm`;
- Arm GNU bare-metal toolchain, especially `arm-none-eabi-gcc`;
- CMake and Ninja on Windows when using the PowerShell helper;
- an already-built ELF if running from Linux/WSL without the Arm toolchain.

The helper scripts build on first use when the selected ELF is missing.

```powershell
.\scripts\run-qemu.ps1 -Help
```

```sh
./scripts/run-qemu.sh --help
```

Quit QEMU with `Ctrl-A x`.

## lm3s6965evb

`lm3s6965evb` is the current Cortex-M3 workhorse target. It is the most complete
QEMU firmware profile today and is selected by `MIPOS_TARGET_QEMU_ARM`.

The build is defined in `cmake/qemu-arm.cmake`. Firmware images share the same
core sources:

- `mipos_kernel.c`, `mipos_task.c`, `mipos_timer.c`, `mipos_queue.c`;
- `mipos_malloc.c`, `mipos_mpool.c`;
- `mipos/arch/qemu-arm/mipos_bsp_qemu_arm.c`;
- `startup_qemu_arm.s` and `vectors_qemu_arm.c`.

### Firmware matrix

| Script name | ELF | Extra modules | Purpose |
|---|---|---|---|
| `helloworld` | `mipos-arm-qemu.elf` | core only | boot mipOS and print periodic ticks |
| `console` | `mipos-arm-qemu-console.elf` | `mipos_console.c` | interactive serial CLI |
| `filesystem` | `mipos-arm-qemu-filesystem.elf` | console + tiny filesystem | RAM-backed filesystem demo |
| `fatfs` | `mipos-arm-qemu-fatfs.elf` | console + `mipos_diskio` + FatFs + semihosting | FAT filesystem over host-backed disk image |
| `net-console` | `mipos-arm-qemu-net-console.elf` | console + lwIP + mipOS netif | interactive IPv4/ARP/ICMP console with in-memory peer |
| `net-selftest` | `mipos-arm-qemu-net-selftest.elf` | lwIP + mipOS netif | non-interactive ARP/ICMP smoke test |

Run from PowerShell:

```powershell
.\scripts\run-qemu.ps1 filesystem
.\scripts\run-qemu.ps1 console
.\scripts\run-qemu.ps1 fatfs
.\scripts\run-qemu.ps1 net-console
.\scripts\run-qemu.ps1 net-selftest
.\scripts\run-qemu.ps1 helloworld
```

Run from Linux or WSL after the firmware exists, or with the Arm bare-metal
toolchain installed if a rebuild is needed:

```sh
./scripts/run-qemu.sh net-console
```

### Direct build

```sh
cmake -S . -B build-qemu-arm \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake \
      -DMIPOS_TARGET_QEMU_ARM=ON
cmake --build build-qemu-arm --parallel
```

Network firmware is optional at configure time:

```sh
cmake -S . -B build-qemu-arm \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake \
      -DMIPOS_TARGET_QEMU_ARM=ON \
      -DMIPOS_QEMU_ARM_NET_CONSOLE=ON \
      -DMIPOS_QEMU_ARM_NET_SELFTEST=ON
cmake --build build-qemu-arm --parallel
```


### FatFs Host-Disk Path

The `fatfs` firmware mounts `build-qemu-arm/qemu-fatfs.img` as FatFs drive `0:`
through QEMU semihosting. The firmware registers
`mipos_qemu_hostdisk_driver()`, then calls `f_mount(&g_fat_fs, "0:", 1)` and
exposes `ls`, `cd`, `pwd`, `cat`, and `write` console commands.

Use this target when changing `mipos_diskio`, FatFs glue, semihosting, or future
block-device work. The detailed storage path is documented in
[Storage And FatFs](Storage-FatFs).

### Expected network self-test

```text
mipOS QEMU lwIP ARP/ICMP self-test
ARP reply OK
ICMP echo reply OK
PASS
```

The self-test uses an in-memory peer, not an emulated NIC. That is deliberate:
it validates the mipOS/lwIP `netif` boundary in bare-metal firmware before a
real board or QEMU NIC driver is introduced.

## vexpress-a9

`vexpress-a9` is the didactic historical board profile. It is useful for
classic ARMv7-A bring-up with fixed MMIO devices and is selected by
`MIPOS_TARGET_QEMU_VEXPRESS_A9`.

The build is defined in `cmake/qemu-vexpress-a9.cmake`. The current firmware
uses:

- `startup_qemu_vexpress_a9.s` for vectors, mode stacks, `.data`, and `.bss`;
- `linker_qemu_vexpress_a9.ld` for the RAM image layout;
- `mipos_bsp_qemu_vexpress_a9.c` for PL011 UART output and BSP glue;
- the normal mipOS kernel, queue, malloc, and memory-pool sources.

Current status:

- PL011 UART output works.
- ARMv7-A startup initializes stacks, `.data`, `.bss`, and vectors.
- mipOS starts a root task.
- Critical sections use CPSR interrupt masking.
- Timer/GIC, PL181 SD, and LAN9118 Ethernet are intentionally future steps.

Run from PowerShell:

```powershell
.\scripts\run-qemu.ps1 vexpress-helloworld
```

Run directly:

```sh
cmake -S . -B build-qemu-vexpress-a9 \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake \
      -DMIPOS_TARGET_QEMU_VEXPRESS_A9=ON
cmake --build build-qemu-vexpress-a9 --parallel
qemu-system-arm -M vexpress-a9 \
    -kernel build-qemu-vexpress-a9/mipos-arm-qemu-vexpress-a9.elf \
    -nographic -serial mon:stdio
```

Expected smoke output:

```text
mipOS QEMU vexpress-a9 port booting...
vexpress-a9 root task running
timer/GIC are intentionally deferred to the next step
```

### vexpress-a9 bring-up roadmap

| Step | Device/subsystem | Goal |
|---|---|---|
| 1 | PL011 UART | done: banner and root task output |
| 2 | GIC + private/generic timer | real scheduler tick instead of epoch placeholder |
| 3 | Console RX | interactive mipOS console on PL011 |
| 4 | PL181 SD/MMC | block device model for FatFs experiments |
| 5 | LAN9118 Ethernet | replace in-memory peer with real emulated Ethernet frames |
| 6 | lwIP console | ARP, ping, UDP/TCP diagnostics on the emulated NIC |

This target is intentionally educational. It exposes the work that a real board
port needs: startup, interrupt controller, timer, block device, and network
controller, one layer at a time.

## virt

`virt` is the planned functional and CI-oriented QEMU board profile. The intent
is to use QEMU's current virtual ARM platform rather than historical evaluation
board devices.

Planned order:

1. PL011 UART boot.
2. ARM generic timer and GIC interrupts.
3. mipOS console firmware.
4. virtio-mmio block device for filesystem tests.
5. virtio-mmio network device for lwIP ARP, ICMP, UDP, TCP, DHCP, and DNS tests.
6. Optional device-tree parsing after the fixed bootstrap path is stable.

The expected advantage over `vexpress-a9` is automation. `virt` should become
the target for repeatable CI tests such as:

- boot banner appears;
- scheduler tick advances;
- console accepts commands;
- block device can mount and write a FAT image;
- lwIP can answer ARP and ICMP through a real QEMU network path.

## Debugging

Both launch scripts support a halted QEMU with a GDB server on `:1234`.

```powershell
.\scripts\run-qemu.ps1 helloworld -Gdb
```

```sh
./scripts/run-qemu.sh helloworld --gdb
```

Then connect with `arm-none-eabi-gdb` and `target remote :1234`.

## Choosing a QEMU profile

- Use `lm3s6965evb` when you want the most complete mipOS/QEMU behavior today.
- Use `net-console` and `net-selftest` when changing lwIP adapter code.
- Use `fatfs` when changing `mipos_diskio`, FatFs glue, or host-disk behavior.
- Use `vexpress-a9` when working on ARMv7-A startup, mode stacks, and classic
  board devices.
- Use `virt` for new CI-oriented block/network work once its first UART/timer
  stage exists.

## Related pages

- [Supported Targets](Supported-Targets)
- [Build and Test](Build-and-Test)
- [Networking](Networking)
- [Storage And FatFs](Storage-FatFs)
- [Porting Guide](Porting-Guide)
