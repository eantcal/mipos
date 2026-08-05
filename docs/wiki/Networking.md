# Networking

mipOS networking is optional and currently based on lwIP. The integration keeps
networking outside the kernel core: mipOS provides scheduling and timing, while
lwIP owns protocol state and a small mipOS adapter moves packets between lwIP
and the selected simulated or QEMU link backend.

## Supported protocol coverage

The current lwIP integration has smoke coverage for:

- IPv4 addressing and routing diagnostics.
- ARP resolution.
- ICMP echo request/reply tests.
- UDP send path.
- TCP connect attempt path.
- DHCP client control and status plumbing.
- DNS server configuration and name lookup plumbing.

IPv6 is not the current focus of the mipOS integration; the implemented command
surface and tests are IPv4-oriented.

## Host simulator backends

The simulator can exercise lwIP through host networking backends:

- Windows Npcap adapter mode for packet capture experiments.
- Windows TAP-Windows direct TAP mode for Ethernet/ARP tests.
- Linux TAP direct mode for Ethernet/ARP tests.
- Optional host NAT setup for sending mipOS traffic beyond the TAP subnet.

Typical Windows TAP run:

```powershell
.\scripts\run-net-pcap.ps1 -EnsureTap
```

Typical Linux TAP run:

```sh
bash scripts/run-net-pcap.sh --ensure-tap
```

Linux NAT helper:

```sh
bash scripts/run-net-pcap.sh --ensure-tap --enable-nat
```

## QEMU firmware tests

The QEMU ARM network console keeps the driver boundary small by using an
in-memory Ethernet peer. This validates the mipOS/lwIP adapter in bare-metal
firmware before a real emulated NIC backend is added.

Interactive console:

```powershell
.\scripts\run-qemu.ps1 net-console
```

Useful commands inside QEMU:

```text
mipOS-net> ip
mipOS-net> peer
mipOS-net> route
mipOS-net> arp
mipOS-net> ping
mipOS-net> stats
```

Non-interactive self-test:

```powershell
.\scripts\run-qemu.ps1 net-selftest
```

Expected self-test result includes:

```text
ARP reply OK
ICMP echo reply OK
PASS
```
## Architecture notes

The current networking work is intentionally layered:

- mipOS kernel primitives provide cooperative execution and timers.
- The lwIP adapter exposes a `netif` to lwIP.
- Host backends or QEMU peers provide Ethernet frames.
- Console commands expose diagnostics without making networking mandatory for
  small builds.

This makes the network stack selectable and keeps the board driver model small
enough to replace as QEMU and physical targets mature.

## Related pages

- [Architecture](Architecture)
- [QEMU ARM Targets](QEMU-ARM-Targets)
- [Build and Test](Build-and-Test)