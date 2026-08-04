# QEMU virt port roadmap

This directory is reserved for the functional ARM QEMU port based on the `virt`
machine. It is intentionally kept separate from the didactic `vexpress-a9` port:

- `vexpress-a9` is useful for learning classic ARM board bring-up with fixed
  MMIO devices such as PL011, PL181, and LAN9118.
- `virt` is the better long-term CI and feature-validation target because QEMU
  keeps it current and it exposes modern, configurable devices.

Planned bring-up order:

1. UART-only boot on PL011.
2. ARM generic timer plus GIC interrupt delivery.
3. mipOS console firmware.
4. virtio-mmio block device for filesystem tests.
5. virtio-mmio network device for lwIP ARP, ICMP, UDP, TCP, DHCP, and DNS tests.
6. Optional device-tree parsing once the fixed bootstrap path is stable.