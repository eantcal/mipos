# QEMU ARM Targets

mipOS keeps three QEMU ARM profiles side by side. They have different purposes
and should not be treated as interchangeable boards.

## lm3s6965evb

`lm3s6965evb` is the current Cortex-M3 workhorse target. It is the most complete
QEMU firmware profile today.

Supported firmware images include:

- `filesystem`: interactive mipOS tiny filesystem demo.
- `console`: interactive mipOS console.
- `fatfs`: FatFs over a host-backed semihosted disk image.
- `helloworld`: timer tick demo.
- `net-console`: interactive lwIP console with an in-memory Ethernet peer.
- `net-selftest`: non-interactive ARP/ICMP self-test.

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

## vexpress-a9

`vexpress-a9` is the didactic historical board profile. It is useful for
classic ARMv7-A bring-up with fixed MMIO devices.

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

Expected smoke output:

```text
mipOS QEMU vexpress-a9 port booting...
vexpress-a9 root task running
timer/GIC are intentionally deferred to the next step
```

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
## Related pages

- [Supported Targets](Supported-Targets.md)
- [Build and Test](Build-and-Test.md)
- [Porting Guide](Porting-Guide.md)