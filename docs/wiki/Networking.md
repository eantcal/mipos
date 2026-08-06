# Networking

mipOS networking is optional and currently based on lwIP. The integration keeps
networking outside the kernel core: mipOS provides scheduling and timing, while
lwIP owns protocol state and a small mipOS adapter moves Ethernet frames between
lwIP and the selected simulated or QEMU link backend.

The implementation is under `mipos/net/lwip/`, with host examples under
`examples/simu/net_pcap` and `examples/simu/net_tap`, and QEMU firmware examples
under `examples/qemu-arm/net-console` and `examples/qemu-arm/net-selftest`.

## Supported Protocol Coverage

The current lwIP integration has smoke coverage for:

- IPv4 addressing and routing diagnostics.
- ARP resolution.
- ICMP echo request/reply tests.
- UDP send path.
- TCP connect attempt path.
- DHCP client control and status plumbing.
- DNS server configuration and name lookup plumbing.

IPv6 is not the current focus of the mipOS integration. The command surface,
`mipos_lwip_netif` adapter, TAP/pcap examples, and tests are IPv4/Ethernet
oriented.

## Build Selection

The default build does not include lwIP. Enable it explicitly:

```sh
cmake -S . -B build-net -DMIPOS_NET_STACK=lwip
```

Host link backends are selected separately:

```sh
cmake -S . -B build-net-tap -DMIPOS_NET_STACK=lwip -DMIPOS_NET_BACKEND=tap
cmake -S . -B build-net-pcap -DMIPOS_NET_STACK=lwip -DMIPOS_NET_BACKEND=pcap
```

`cmake/lwip.cmake` builds `mipos_lwip` from the adapter plus the fetched lwIP
core. `cmake/lwip-fetch.cmake` selects the protocol files used by this port:
`etharp`, `icmp`, `udp`, `tcp`, `dhcp`, `dns`, `netif`, and `ethernet`.

## Adapter Boundary

The central abstraction is `mipos_lwip_netif_t` in
`mipos/net/lwip/mipos_lwip_netif.h`:

```c
typedef err_t (*mipos_lwip_link_output_fn)(void* ctx,
                                           const uint8_t* frame,
                                           uint16_t frame_len);

typedef struct mipos_lwip_netif {
    struct netif netif;
    uint8_t hwaddr[6];
    mipos_lwip_link_output_fn link_output;
    void* link_output_ctx;
} mipos_lwip_netif_t;
```

lwIP owns `struct netif`. mipOS adds the hardware address and one callback used
to send complete Ethernet frames to the active backend. This keeps the driver
model small: TAP, pcap, QEMU in-memory peers, and future NIC drivers all satisfy
the same `link_output(ctx, frame, len)` contract.

## Netif Setup

`mipos_lwip_netif_open` creates the lwIP `netif`, installs the adapter callbacks,
sets the interface as default, and marks it up:

```c
added = netif_add(&iface->netif,
                  ipaddr,
                  netmask,
                  gateway,
                  iface,
                  mipos_lwip_low_level_init,
                  ethernet_input);

netif_set_default(&iface->netif);
netif_set_up(&iface->netif);
netif_set_link_up(&iface->netif);
```

The low-level initializer gives the interface its lwIP shape:

```c
netif->name[0] = 'm';
netif->name[1] = 'p';
netif->output = etharp_output;
netif->linkoutput = mipos_lwip_low_level_output;
netif->mtu = 1500;
netif->hwaddr_len = 6;
netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
```

That is why ARP is visible and testable: lwIP routes IPv4 output through
`etharp_output`, which resolves Ethernet destinations and eventually calls the
link output callback.

## TX And RX Flow

Transmit flow:

1. lwIP creates one or more `pbuf` segments.
2. `mipos_lwip_low_level_output` copies the packet chain into a contiguous
   Ethernet frame buffer.
3. The selected backend callback sends the frame.

```c
copied = pbuf_copy_partial(p, frame, p->tot_len, 0);
if (copied != p->tot_len) {
    return ERR_IF;
}

return iface->link_output(iface->link_output_ctx, frame, copied);
```

Receive flow:

1. backend reads an Ethernet frame from TAP, pcap, or QEMU peer;
2. backend calls `mipos_lwip_netif_input`;
3. the adapter allocates a `PBUF_RAW`, copies the frame, and passes it to
   `iface->netif.input`.

```c
p = pbuf_alloc(PBUF_RAW, frame_len, PBUF_POOL);
if (!p) {
    return ERR_MEM;
}

if (pbuf_take(p, frame, frame_len) != ERR_OK) {
    pbuf_free(p);
    return ERR_MEM;
}

err = iface->netif.input(p, &iface->netif);
```

This explicit copy is acceptable for the current simulator and QEMU tests. A
future physical NIC driver can optimize the boundary after correctness is stable.

## Timeouts

The adapter is `NO_SYS=1`, so applications must poll lwIP timeouts:

```c
void mipos_lwip_netif_poll(mipos_lwip_netif_t* iface)
{
    (void)iface;
    sys_check_timeouts();
}
```

Host examples call this in their main loop. DHCP, TCP retransmission, DNS retry,
and ARP aging all depend on this polling path.

## Host Simulator Backends

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

The direct TAP backend is the best first end-to-end test because it preserves
Ethernet framing and ARP. pcap mode is useful for adapter experiments and packet
capture/injection, but TAP is the normal path when the host kernel should answer
ARP and ICMP.

## Network Console Commands

The host TAP/pcap examples expose a small mipOS network console. The command
surface is implemented in `examples/simu/net_tap/main.c` and
`examples/simu/net_pcap/main.c`; QEMU has a smaller firmware version under
`examples/qemu-arm/net-console/main.c`.

Common commands:

```text
ip              show mipOS IP, netmask, gateway
route [ip]      show routes or route selected for an IP
arp [ip]        inspect or request ARP resolution
ping ip         send one ICMP echo request
udp ip port [s] send one UDP datagram
tcp ip port     start one TCP connect attempt
dhcp [cmd]      DHCP start, stop, or status
dns server ip   set DNS server
dns name        resolve a DNS name
quiet           disable packet log
verbose         enable packet log
stats           show RX/TX counters
```

The route command is intentionally diagnostic. It shows whether a destination is
connected or must go through the configured gateway before packet debugging
starts.

## Packet Logging

Packet logs are disabled by default in the host console examples. The runtime
command toggles the same `app->verbose` flag used by startup options:

```c
} else if (strcmp(line, "quiet") == 0) {
    app->verbose = 0;
    printf("packet log disabled\n");
} else if (strcmp(line, "verbose") == 0) {
    app->verbose = 1;
    printf("packet log enabled\n");
}
```

This keeps normal runs readable while preserving the ability to debug ARP, ICMP,
UDP, and multicast traffic without rebuilding.

## Tests

`tests/mipos_lwip_tests.cpp` validates the adapter and selected lwIP protocol
paths without requiring a host TAP device. The test captures frames emitted by
`link_output` and injects crafted frames through `mipos_lwip_netif_input`.

Covered paths include:

- ARP reply to a host request;
- ICMP echo reply to an injected echo request;
- UDP frame emission through `udp_sendto`;
- TCP SYN path through `tcp_connect`;
- TCP listen setup;
- DNS query emission through `dns_gethostbyname`;
- DHCP discover emission through `dhcp_start`.

Representative test code:

```c
ASSERT_EQ(ERR_OK, mipos_lwip_netif_input(&iface,
                                         make_arp_request().data(),
                                         42));

ASSERT_EQ(ERR_OK, udp_sendto(udp, udp_payload, &host_addr, 12001));
ASSERT_EQ(ERR_OK, dhcp_start(&iface.netif));
```

These are protocol smoke tests, not throughput or full conformance tests.

## QEMU Firmware Tests

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

The QEMU self-test exercises the same adapter calls as the host tests:

```c
if (mipos_lwip_netif_input(&iface, input, 42) != ERR_OK ||
    !check_icmp_reply()) {
    uart_puts("FAIL: ICMP echo reply\n");
}
```

## Architecture Notes

The current networking work is intentionally layered:

- mipOS kernel primitives provide cooperative execution and timers.
- The lwIP adapter exposes a `netif` to lwIP.
- Host backends or QEMU peers provide Ethernet frames.
- Console commands expose diagnostics without making networking mandatory for
  small builds.

This makes the network stack selectable and keeps the board driver model small
enough to replace as QEMU and physical targets mature.

## Current Limits

- IPv4/Ethernet is the supported path; IPv6 is not integrated.
- `NO_SYS=1` means no lwIP socket or netconn thread model.
- Host NAT setup is helper-script driven and depends on host firewall/routing
  policy.
- QEMU networking currently validates the adapter with an in-memory peer, not a
  real emulated NIC such as Stellaris Ethernet, LAN9118, or virtio-net.
- The command console is diagnostic, not a production shell protocol.

## Related Pages

- [Architecture](Architecture)
- [QEMU ARM Targets](QEMU-ARM-Targets)
- [Build and Test](Build-and-Test)
- [Porting Guide](Porting-Guide)