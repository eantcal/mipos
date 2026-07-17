#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "mipos_lwip_netif.h"

#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#include <winioctl.h>
#else
#error "example-net-tap currently supports Windows/TAP-Windows only"
#endif

#include "lwip/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/prot/icmp.h"
#include "lwip/raw.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"

#define TAP_CONTROL_CODE(request, method) \
    CTL_CODE(FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)
#define TAP_WIN_IOCTL_SET_MEDIA_STATUS TAP_CONTROL_CODE(6, METHOD_BUFFERED)
#define CONSOLE_LINE_MAX 128
#define CONSOLE_HISTORY_SIZE 16
#define CONSOLE_KEY_EXTENDED_0 0
#define CONSOLE_KEY_EXTENDED_1 224
#define CONSOLE_KEY_UP 72
#define CONSOLE_KEY_DOWN 80

typedef struct app_config {
    const char* guid;
    int verbose;
    uint8_t mac[6];
    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gateway;
} app_config_t;

typedef struct app_context {
    HANDLE tap;
    mipos_lwip_netif_t* iface;
    struct raw_pcb* ping_pcb;
    struct tcp_pcb* tcp_pcb;
    uint8_t mac[6];
    uint8_t ip[4];
    int verbose;
    unsigned int rx_count;
    unsigned int tx_count;
    uint16_t ping_id;
    uint16_t ping_seq;
} app_context_t;

static volatile int g_stop = 0;

static void on_signal(int sig)
{
    (void)sig;
    g_stop = 1;
}

static void usage(const char* exe)
{
    printf("Usage:\n");
    printf("  %s --guid {TAP-INTERFACE-GUID} [--ip 10.77.0.2] "
           "[--netmask 255.255.255.0] [--gateway 10.77.0.1] [--verbose]\n",
           exe);
    printf("\n");
    printf("The helper script resolves the GUID automatically:\n");
    printf("  .\\scripts\\run-net-pcap.ps1 -EnsureTap\n");
}

static void print_prompt(void)
{
    printf("mipOS-net> ");
    fflush(stdout);
}

static void redraw_console_line(const char* line,
                                unsigned int len,
                                unsigned int previous_len)
{
    printf("\rmipOS-net> %s", line);
    if (previous_len > len) {
        for (unsigned int i = len; i < previous_len; ++i) {
            putchar(' ');
        }
        for (unsigned int i = len; i < previous_len; ++i) {
            putchar('\b');
        }
    }
    fflush(stdout);
}

static int parse_ip(const char* text, ip4_addr_t* out)
{
    unsigned int a, b, c, d;

    if (sscanf(text, "%u.%u.%u.%u", &a, &b, &c, &d) != 4 ||
        a > 255 || b > 255 || c > 255 || d > 255) {
        return 0;
    }

    IP4_ADDR(out, a, b, c, d);
    return 1;
}

static int parse_mac(const char* text, uint8_t mac[6])
{
    unsigned int b[6];

    if (sscanf(text, "%x:%x:%x:%x:%x:%x",
               &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) != 6) {
        return 0;
    }

    for (int i = 0; i < 6; ++i) {
        if (b[i] > 0xff) {
            return 0;
        }
        mac[i] = (uint8_t)b[i];
    }

    return 1;
}

static uint16_t read_be16(const uint8_t* data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static void format_ipv4(const uint8_t* ip, char* out, size_t out_size)
{
    snprintf(out, out_size, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}

static int is_icmp_echo(const uint8_t* frame, unsigned int len, uint8_t type)
{
    unsigned int ip_header_len;
    unsigned int total_len;

    if (len < 14 + 20 || read_be16(frame + 12) != 0x0800 || frame[23] != 1) {
        return 0;
    }

    ip_header_len = (unsigned int)(frame[14] & 0x0f) * 4u;
    total_len = read_be16(frame + 16);
    if (ip_header_len < 20 || len < 14 + ip_header_len + 8 ||
        total_len < ip_header_len + 8) {
        return 0;
    }

    return frame[14 + ip_header_len] == type;
}

static void log_frame(const char* dir,
                      const uint8_t* self_ip,
                      const uint8_t* frame,
                      unsigned int len)
{
    uint16_t eth_type;
    char src_ip[16];
    char dst_ip[16];

    if (len < 14) {
        printf("%s frame len=%u\n", dir, len);
        return;
    }

    eth_type = read_be16(frame + 12);

    if (eth_type == 0x0806 && len >= 42) {
        uint16_t op = read_be16(frame + 20);
        format_ipv4(frame + 28, src_ip, sizeof(src_ip));
        format_ipv4(frame + 38, dst_ip, sizeof(dst_ip));

        if (op == 1) {
            printf("%s ARP who-has %s tell %s%s\n",
                   dir,
                   dst_ip,
                   src_ip,
                   memcmp(frame + 38, self_ip, 4) == 0 ? " (for mipOS)" : "");
        } else if (op == 2) {
            printf("%s ARP is-at %s\n", dir, src_ip);
        } else {
            printf("%s ARP op=%u %s -> %s\n", dir, op, src_ip, dst_ip);
        }
    } else if (eth_type == 0x0800 && len >= 34) {
        format_ipv4(frame + 26, src_ip, sizeof(src_ip));
        format_ipv4(frame + 30, dst_ip, sizeof(dst_ip));

        if (is_icmp_echo(frame, len, 8)) {
            printf("%s ICMP echo request %s -> %s\n", dir, src_ip, dst_ip);
        } else if (is_icmp_echo(frame, len, 0)) {
            printf("%s ICMP echo reply %s -> %s\n", dir, src_ip, dst_ip);
        } else {
            printf("%s IPv4 proto=%u %s -> %s\n", dir, frame[23], src_ip, dst_ip);
        }
    } else {
        printf("%s ethertype=0x%04x len=%u\n", dir, eth_type, len);
    }
}

static void init_config(app_config_t* cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    cfg->verbose = 0;
    cfg->mac[0] = 0x02;
    cfg->mac[1] = 0x00;
    cfg->mac[2] = 0x00;
    cfg->mac[3] = 0x00;
    cfg->mac[4] = 0x00;
    cfg->mac[5] = 0x01;
    IP4_ADDR(&cfg->ip, 10, 77, 0, 2);
    IP4_ADDR(&cfg->netmask, 255, 255, 255, 0);
    IP4_ADDR(&cfg->gateway, 10, 77, 0, 1);
}

static int parse_args(int argc, char* argv[], app_config_t* cfg)
{
    init_config(cfg);

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "--guid") == 0 && i + 1 < argc) {
            cfg->guid = argv[++i];
        } else if (strcmp(argv[i], "--ip") == 0 && i + 1 < argc) {
            if (!parse_ip(argv[++i], &cfg->ip)) {
                fprintf(stderr, "invalid --ip\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--netmask") == 0 && i + 1 < argc) {
            if (!parse_ip(argv[++i], &cfg->netmask)) {
                fprintf(stderr, "invalid --netmask\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--gateway") == 0 && i + 1 < argc) {
            if (!parse_ip(argv[++i], &cfg->gateway)) {
                fprintf(stderr, "invalid --gateway\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--mac") == 0 && i + 1 < argc) {
            if (!parse_mac(argv[++i], cfg->mac)) {
                fprintf(stderr, "invalid --mac\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--quiet") == 0) {
            cfg->verbose = 0;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            cfg->verbose = 1;
        } else {
            fprintf(stderr, "unknown or incomplete option: %s\n", argv[i]);
            return 0;
        }
    }

    if (!cfg->guid) {
        fprintf(stderr, "missing --guid\n");
        return 0;
    }

    return 1;
}

static void tap_path_from_guid(const char* guid, char* path, size_t path_size)
{
    if (guid[0] == '{') {
        snprintf(path, path_size, "\\\\.\\Global\\%s.tap", guid);
    } else {
        snprintf(path, path_size, "\\\\.\\Global\\{%s}.tap", guid);
    }
}

static int set_tap_media_status(HANDLE tap, int connected)
{
    unsigned long status = connected ? 1u : 0u;
    unsigned long len = 0;

    return DeviceIoControl(tap,
                           TAP_WIN_IOCTL_SET_MEDIA_STATUS,
                           &status,
                           sizeof(status),
                           &status,
                           sizeof(status),
                           &len,
                           NULL) != 0;
}

static int tap_read_frame(HANDLE tap, uint8_t* frame, unsigned int frame_size, unsigned int* len)
{
    OVERLAPPED ov;
    DWORD got = 0;
    DWORD err;
    BOOL ok;

    memset(&ov, 0, sizeof(ov));
    ov.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (!ov.hEvent) {
        return -1;
    }

    ok = ReadFile(tap, frame, frame_size, &got, &ov);
    if (!ok) {
        err = GetLastError();
        if (err == ERROR_IO_PENDING) {
            DWORD wait = WaitForSingleObject(ov.hEvent, 100);
            if (wait == WAIT_TIMEOUT) {
                CancelIo(tap);
                CloseHandle(ov.hEvent);
                return 0;
            }
            if (wait != WAIT_OBJECT_0 ||
                !GetOverlappedResult(tap, &ov, &got, FALSE)) {
                CloseHandle(ov.hEvent);
                return -1;
            }
        } else {
            CloseHandle(ov.hEvent);
            return -1;
        }
    }

    CloseHandle(ov.hEvent);
    *len = got;
    return 1;
}

static int tap_write_frame(HANDLE tap, const uint8_t* frame, unsigned int len)
{
    OVERLAPPED ov;
    DWORD written = 0;
    DWORD err;
    BOOL ok;

    memset(&ov, 0, sizeof(ov));
    ov.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (!ov.hEvent) {
        return 0;
    }

    ok = WriteFile(tap, frame, len, &written, &ov);
    if (!ok) {
        err = GetLastError();
        if (err == ERROR_IO_PENDING) {
            DWORD wait = WaitForSingleObject(ov.hEvent, 5000);
            if (wait != WAIT_OBJECT_0 ||
                !GetOverlappedResult(tap, &ov, &written, FALSE)) {
                CancelIo(tap);
                CloseHandle(ov.hEvent);
                return 0;
            }
        } else {
            CloseHandle(ov.hEvent);
            return 0;
        }
    }

    CloseHandle(ov.hEvent);
    return written == len;
}

static err_t tap_link_output(void* ctx, const uint8_t* frame, uint16_t frame_len)
{
    app_context_t* app = (app_context_t*)ctx;

    ++app->tx_count;
    if (app->verbose) {
        log_frame("TX", app->ip, frame, frame_len);
    }

    if (!tap_write_frame(app->tap, frame, frame_len)) {
        fprintf(stderr, "TAP write failed: %lu\n", GetLastError());
        return ERR_IF;
    }

    return ERR_OK;
}

static int is_own_ethernet_frame(const uint8_t mac[6],
                                 const unsigned char* frame,
                                 unsigned int len)
{
    return len >= 12 && memcmp(frame + 6, mac, 6) == 0;
}

static void print_ip(const char* label, const ip4_addr_t* ip)
{
    printf("%s%u.%u.%u.%u",
           label,
           ip4_addr1(ip),
           ip4_addr2(ip),
           ip4_addr3(ip),
           ip4_addr4(ip));
}

static void print_ip_only(const ip4_addr_t* ip)
{
    print_ip("", ip);
}

static void format_ip4_addr(const ip4_addr_t* ip, char* out, size_t out_size)
{
    snprintf(out,
             out_size,
             "%u.%u.%u.%u",
             ip4_addr1(ip),
             ip4_addr2(ip),
             ip4_addr3(ip),
             ip4_addr4(ip));
}

static void get_network_addr(ip4_addr_t* out,
                             const ip4_addr_t* ip,
                             const ip4_addr_t* netmask)
{
    out->addr = ip->addr & netmask->addr;
}

static int parse_ip_arg(const char* text, ip4_addr_t* out)
{
    while (*text == ' ' || *text == '\t') {
        ++text;
    }
    return parse_ip(text, out);
}

static uint8_t ping_recv(void* arg,
                         struct raw_pcb* pcb,
                         struct pbuf* p,
                         const ip_addr_t* addr)
{
    app_context_t* app = (app_context_t*)arg;
    uint8_t header[8];
    uint16_t id;
    uint16_t seq;
    const ip4_addr_t* from = ip_2_ip4(addr);

    (void)pcb;

    if (p->tot_len < sizeof(header)) {
        return 0;
    }

    pbuf_copy_partial(p, header, sizeof(header), 0);
    if (header[0] != ICMP_ER) {
        return 0;
    }

    id = read_be16(header + 4);
    seq = read_be16(header + 6);
    if (id != app->ping_id) {
        return 0;
    }

    printf("ping reply from %u.%u.%u.%u seq=%u bytes=%u\n",
           ip4_addr1(from),
           ip4_addr2(from),
           ip4_addr3(from),
           ip4_addr4(from),
           seq,
           p->tot_len);
    print_prompt();

    pbuf_free(p);
    return 1;
}

static void send_ping(app_context_t* app, const ip4_addr_t* dst)
{
    const unsigned int payload_len = 16;
    struct pbuf* p = pbuf_alloc(PBUF_IP,
                                (u16_t)(sizeof(struct icmp_echo_hdr) + payload_len),
                                PBUF_RAM);
    struct icmp_echo_hdr* echo;
    ip_addr_t dst_addr;
    uint8_t* payload;
    err_t err;

    if (!p) {
        printf("ping: pbuf_alloc failed\n");
        return;
    }

    echo = (struct icmp_echo_hdr*)p->payload;
    ICMPH_TYPE_SET(echo, ICMP_ECHO);
    ICMPH_CODE_SET(echo, 0);
    echo->id = lwip_htons(app->ping_id);
    echo->seqno = lwip_htons(++app->ping_seq);

    payload = (uint8_t*)p->payload + sizeof(struct icmp_echo_hdr);
    for (unsigned int i = 0; i < payload_len; ++i) {
        payload[i] = (uint8_t)('a' + (i % 26));
    }

    echo->chksum = 0;
    echo->chksum = inet_chksum(echo, p->len);

    printf("ping %u.%u.%u.%u seq=%u\n",
           ip4_addr1(dst),
           ip4_addr2(dst),
           ip4_addr3(dst),
           ip4_addr4(dst),
           app->ping_seq);

    ip_addr_copy_from_ip4(dst_addr, *dst);
    err = raw_sendto(app->ping_pcb, p, &dst_addr);
    if (err != ERR_OK) {
        printf("ping: raw_sendto failed: %d\n", err);
    }

    pbuf_free(p);
}

static void show_arp(app_context_t* app, const ip4_addr_t* ip)
{
    const struct eth_addr* eth = NULL;
    const ip4_addr_t* found_ip = NULL;
    int index = etharp_find_addr(&app->iface->netif, ip, &eth, &found_ip);

    if (index >= 0 && eth && found_ip) {
        printf("arp %u.%u.%u.%u -> %02x:%02x:%02x:%02x:%02x:%02x\n",
               ip4_addr1(found_ip),
               ip4_addr2(found_ip),
               ip4_addr3(found_ip),
               ip4_addr4(found_ip),
               eth->addr[0],
               eth->addr[1],
               eth->addr[2],
               eth->addr[3],
               eth->addr[4],
               eth->addr[5]);
    } else {
        printf("arp %u.%u.%u.%u unresolved; sending ARP request\n",
               ip4_addr1(ip),
               ip4_addr2(ip),
               ip4_addr3(ip),
               ip4_addr4(ip));
        etharp_request(&app->iface->netif, ip);
    }
}

static void show_route_table(const app_config_t* cfg)
{
    ip4_addr_t network;
    char destination[16];
    char gateway[16];
    char netmask[16];

    get_network_addr(&network, &cfg->ip, &cfg->netmask);
    format_ip4_addr(&network, destination, sizeof(destination));
    format_ip4_addr(&cfg->netmask, netmask, sizeof(netmask));
    format_ip4_addr(&cfg->gateway, gateway, sizeof(gateway));

    printf("%-16s %-16s %-16s %-6s %s\n",
           "destination",
           "gateway",
           "netmask",
           "iface",
           "type");
    printf("%-16s %-16s %-16s %-6s %s\n",
           destination,
           "0.0.0.0",
           netmask,
           "mip0",
           "connected");
    printf("%-16s %-16s %-16s %-6s %s\n",
           "0.0.0.0",
           gateway,
           "0.0.0.0",
           "mip0",
           "default");
}

static void show_route_for(const app_config_t* cfg, const ip4_addr_t* dst)
{
    const ip4_addr_t* next_hop = dst;
    const char* kind = "direct";
    char destination[16];
    char gateway[16];

    if (!ip4_addr_net_eq(dst, &cfg->ip, &cfg->netmask)) {
        next_hop = &cfg->gateway;
        kind = "default";
    }

    format_ip4_addr(dst, destination, sizeof(destination));
    format_ip4_addr(next_hop, gateway, sizeof(gateway));

    printf("%-16s %-16s %-6s %s\n",
           "destination",
           "next-hop",
           "iface",
           "type");
    printf("%-16s %-16s %-6s %s\n",
           destination,
           gateway,
           "mip0",
           kind);
}

static void send_udp_datagram(app_context_t* app, const char* args)
{
    char ip_text[32];
    char payload[80] = "mipOS";
    unsigned int port;
    ip4_addr_t dst_ip;
    ip_addr_t dst_addr;
    struct udp_pcb* pcb;
    struct pbuf* p;
    err_t err;
    int parsed;

    (void)app;

    parsed = sscanf(args, "%31s %u %79[^\n]", ip_text, &port, payload);
    if (parsed < 2 || port > 65535 || !parse_ip(ip_text, &dst_ip)) {
        printf("usage: udp <ip> <port> [payload]\n");
        return;
    }

    pcb = udp_new();
    if (!pcb) {
        printf("udp: udp_new failed\n");
        return;
    }

    p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)strlen(payload), PBUF_RAM);
    if (!p) {
        udp_remove(pcb);
        printf("udp: pbuf_alloc failed\n");
        return;
    }

    pbuf_take(p, payload, (u16_t)strlen(payload));
    ip_addr_copy_from_ip4(dst_addr, dst_ip);
    err = udp_sendto(pcb, p, &dst_addr, (u16_t)port);
    printf("udp ");
    print_ip_only(&dst_ip);
    printf(":%u len=%u", port, (unsigned int)strlen(payload));
    if (err != ERR_OK) {
        printf(" failed: %d", err);
    }
    printf("\n");

    pbuf_free(p);
    udp_remove(pcb);
}

static err_t tcp_connected(void* arg, struct tcp_pcb* pcb, err_t err)
{
    (void)arg;
    (void)pcb;

    printf("tcp connected: %d\n", err);
    print_prompt();
    return ERR_OK;
}

static void tcp_error(void* arg, err_t err)
{
    app_context_t* app = (app_context_t*)arg;

    app->tcp_pcb = NULL;
    printf("tcp error: %d\n", err);
    print_prompt();
}

static void start_tcp_connect(app_context_t* app, const char* args)
{
    char ip_text[32];
    unsigned int port;
    ip4_addr_t dst_ip;
    ip_addr_t dst_addr;
    err_t err;

    if (sscanf(args, "%31s %u", ip_text, &port) != 2 ||
        port > 65535 || !parse_ip(ip_text, &dst_ip)) {
        printf("usage: tcp <ip> <port>\n");
        return;
    }

    if (app->tcp_pcb) {
        tcp_abort(app->tcp_pcb);
        app->tcp_pcb = NULL;
    }

    app->tcp_pcb = tcp_new();
    if (!app->tcp_pcb) {
        printf("tcp: tcp_new failed\n");
        return;
    }

    tcp_arg(app->tcp_pcb, app);
    tcp_err(app->tcp_pcb, tcp_error);
    ip_addr_copy_from_ip4(dst_addr, dst_ip);
    err = tcp_connect(app->tcp_pcb, &dst_addr, (u16_t)port, tcp_connected);
    printf("tcp connect ");
    print_ip_only(&dst_ip);
    printf(":%u", port);
    if (err != ERR_OK) {
        printf(" failed: %d", err);
        tcp_abort(app->tcp_pcb);
        app->tcp_pcb = NULL;
    }
    printf("\n");
}

static void handle_dhcp_command(app_context_t* app, const char* args)
{
    while (*args == ' ' || *args == '\t') {
        ++args;
    }

    if (strcmp(args, "start") == 0) {
        err_t err = dhcp_start(&app->iface->netif);
        printf("dhcp start: %d\n", err);
    } else if (strcmp(args, "stop") == 0) {
        dhcp_stop(&app->iface->netif);
        printf("dhcp stopped\n");
    } else if (strcmp(args, "status") == 0 || *args == 0) {
        print_ip("ip:       ", netif_ip4_addr(&app->iface->netif));
        printf("\n");
        print_ip("netmask:  ", netif_ip4_netmask(&app->iface->netif));
        printf("\n");
        print_ip("gateway:  ", netif_ip4_gw(&app->iface->netif));
        printf("\n");
    } else {
        printf("usage: dhcp [start|stop|status]\n");
    }
}

static void dns_found(const char* name, const ip_addr_t* ipaddr, void* arg)
{
    (void)arg;

    if (ipaddr) {
        const ip4_addr_t* ip4 = ip_2_ip4(ipaddr);
        printf("dns %s -> ", name);
        print_ip_only(ip4);
        printf("\n");
    } else {
        printf("dns %s failed\n", name);
    }
    print_prompt();
}

static void handle_dns_command(const char* args)
{
    char first[128];
    char second[32];

    while (*args == ' ' || *args == '\t') {
        ++args;
    }

    if (sscanf(args, "server %31s", second) == 1) {
        ip4_addr_t server_ip;
        ip_addr_t server_addr;

        if (!parse_ip(second, &server_ip)) {
            printf("usage: dns server <ip>\n");
            return;
        }

        ip_addr_copy_from_ip4(server_addr, server_ip);
        dns_setserver(0, &server_addr);
        printf("dns server ");
        print_ip_only(&server_ip);
        printf("\n");
        return;
    }

    if (sscanf(args, "%127s", first) != 1) {
        printf("usage: dns server <ip> | dns <name>\n");
        return;
    }

    ip_addr_t resolved;
    err_t err = dns_gethostbyname(first, &resolved, dns_found, NULL);
    if (err == ERR_OK) {
        printf("dns %s -> ", first);
        print_ip_only(ip_2_ip4(&resolved));
        printf("\n");
    } else if (err == ERR_INPROGRESS) {
        printf("dns %s pending\n", first);
    } else {
        printf("dns %s failed: %d\n", first, err);
    }
}

static void execute_command(app_context_t* app,
                            const app_config_t* cfg,
                            const char* line)
{
    ip4_addr_t arg_ip;

    while (*line == ' ' || *line == '\t') {
        ++line;
    }

    if (*line == 0) {
        return;
    }

    if (strcmp(line, "help") == 0 || strcmp(line, "?") == 0) {
        printf("commands:\n");
        printf("  ip              show mipOS/Windows addresses\n");
        printf("  route [ip]      show routes or route selected for an IP\n");
        printf("  arp [ip]        show/resolve an ARP entry, default gateway\n");
        printf("  ping [ip]       send one ICMP echo request, default gateway\n");
        printf("  udp ip port [s] send one UDP datagram\n");
        printf("  tcp ip port     start one TCP connect attempt\n");
        printf("  dhcp [cmd]      DHCP start, stop, or status\n");
        printf("  dns server ip   set DNS server\n");
        printf("  dns name        resolve a DNS name\n");
        printf("  stats           show RX/TX counters\n");
        printf("  quiet           disable packet log\n");
        printf("  verbose         enable packet log\n");
        printf("  quit            exit\n");
    } else if (strcmp(line, "ip") == 0) {
        print_ip("mipOS IP: ", &cfg->ip);
        printf("\n");
        print_ip("gateway:  ", &cfg->gateway);
        printf("\n");
    } else if (strncmp(line, "route", 5) == 0 &&
               (line[5] == 0 || line[5] == ' ' || line[5] == '\t')) {
        if (line[5] == 0) {
            show_route_table(cfg);
        } else if (!parse_ip_arg(line + 5, &arg_ip)) {
            printf("usage: route [ip]\n");
            return;
        } else {
            show_route_for(cfg, &arg_ip);
        }
    } else if (strncmp(line, "arp", 3) == 0 &&
               (line[3] == 0 || line[3] == ' ' || line[3] == '\t')) {
        if (line[3] == 0) {
            arg_ip = cfg->gateway;
        } else if (!parse_ip_arg(line + 3, &arg_ip)) {
            printf("usage: arp [ip]\n");
            return;
        }
        show_arp(app, &arg_ip);
    } else if (strncmp(line, "ping", 4) == 0 &&
               (line[4] == 0 || line[4] == ' ' || line[4] == '\t')) {
        if (line[4] == 0) {
            arg_ip = cfg->gateway;
        } else if (!parse_ip_arg(line + 4, &arg_ip)) {
            printf("usage: ping [ip]\n");
            return;
        }
        send_ping(app, &arg_ip);
    } else if (strncmp(line, "udp", 3) == 0 &&
               (line[3] == 0 || line[3] == ' ' || line[3] == '\t')) {
        send_udp_datagram(app, line + 3);
    } else if (strncmp(line, "tcp", 3) == 0 &&
               (line[3] == 0 || line[3] == ' ' || line[3] == '\t')) {
        start_tcp_connect(app, line + 3);
    } else if (strncmp(line, "dhcp", 4) == 0 &&
               (line[4] == 0 || line[4] == ' ' || line[4] == '\t')) {
        handle_dhcp_command(app, line + 4);
    } else if (strncmp(line, "dns", 3) == 0 &&
               (line[3] == 0 || line[3] == ' ' || line[3] == '\t')) {
        handle_dns_command(line + 3);
    } else if (strcmp(line, "stats") == 0) {
        printf("RX=%u TX=%u\n", app->rx_count, app->tx_count);
    } else if (strcmp(line, "quiet") == 0) {
        app->verbose = 0;
        printf("packet log disabled\n");
    } else if (strcmp(line, "verbose") == 0) {
        app->verbose = 1;
        printf("packet log enabled\n");
    } else if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
        g_stop = 1;
    } else {
        printf("unknown command: %s\n", line);
    }
}

static void poll_console(app_context_t* app, const app_config_t* cfg)
{
    static char line[CONSOLE_LINE_MAX];
    static char history[CONSOLE_HISTORY_SIZE][CONSOLE_LINE_MAX];
    static unsigned int len = 0;
    static unsigned int history_count = 0;
    static unsigned int history_pos = 0;

    while (_kbhit()) {
        int ch = _getch();

        if (ch == '\r' || ch == '\n') {
            putchar('\n');
            line[len] = 0;
            if (len > 0 &&
                (history_count == 0 ||
                 strcmp(history[(history_count - 1) % CONSOLE_HISTORY_SIZE],
                        line) != 0)) {
                strncpy(history[history_count % CONSOLE_HISTORY_SIZE],
                        line,
                        sizeof(history[0]) - 1);
                history[history_count % CONSOLE_HISTORY_SIZE]
                       [sizeof(history[0]) - 1] = 0;
                ++history_count;
            }
            history_pos = history_count;
            execute_command(app, cfg, line);
            len = 0;
            line[0] = 0;
            if (!g_stop) {
                print_prompt();
            }
        } else if (ch == CONSOLE_KEY_EXTENDED_0 ||
                   ch == CONSOLE_KEY_EXTENDED_1) {
            unsigned int visible_count =
              history_count < CONSOLE_HISTORY_SIZE ? history_count
                                                   : CONSOLE_HISTORY_SIZE;
            int ext = _getch();

            if (visible_count == 0) {
                continue;
            }

            if (ext == CONSOLE_KEY_UP) {
                unsigned int oldest = history_count - visible_count;
                unsigned int previous_len = len;

                if (history_pos > oldest) {
                    --history_pos;
                }

                strncpy(line,
                        history[history_pos % CONSOLE_HISTORY_SIZE],
                        sizeof(line) - 1);
                line[sizeof(line) - 1] = 0;
                len = (unsigned int)strlen(line);
                redraw_console_line(line, len, previous_len);
            } else if (ext == CONSOLE_KEY_DOWN) {
                unsigned int previous_len = len;

                if (history_pos < history_count) {
                    ++history_pos;
                }

                if (history_pos == history_count) {
                    len = 0;
                    line[0] = 0;
                } else {
                    strncpy(line,
                            history[history_pos % CONSOLE_HISTORY_SIZE],
                            sizeof(line) - 1);
                    line[sizeof(line) - 1] = 0;
                    len = (unsigned int)strlen(line);
                }
                redraw_console_line(line, len, previous_len);
            }
        } else if (ch == '\b') {
            if (len > 0) {
                --len;
                line[len] = 0;
                printf("\b \b");
            }
        } else if (ch >= 32 && ch < 127 && len + 1 < sizeof(line)) {
            line[len++] = (char)ch;
            line[len] = 0;
            history_pos = history_count;
            putchar(ch);
        }
    }
}

int main(int argc, char* argv[])
{
    app_config_t cfg;
    app_context_t app;
    mipos_lwip_netif_t iface;
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gateway;
    char tap_path[256];
    uint8_t frame[65536];

    if (!parse_args(argc, argv, &cfg)) {
        usage(argv[0]);
        return 2;
    }

    tap_path_from_guid(cfg.guid, tap_path, sizeof(tap_path));

    memset(&app, 0, sizeof(app));
    app.verbose = cfg.verbose;
    app.ping_id = 0x4d50;
    memcpy(app.mac, cfg.mac, sizeof(app.mac));
    app.ip[0] = ip4_addr1(&cfg.ip);
    app.ip[1] = ip4_addr2(&cfg.ip);
    app.ip[2] = ip4_addr3(&cfg.ip);
    app.ip[3] = ip4_addr4(&cfg.ip);

    app.tap = CreateFileA(tap_path,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          0,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
                          0);
    if (app.tap == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "failed to open TAP device %s: %lu\n", tap_path, GetLastError());
        return 1;
    }

    if (!set_tap_media_status(app.tap, 1)) {
        fprintf(stderr, "failed to set TAP media status: %lu\n", GetLastError());
        CloseHandle(app.tap);
        return 1;
    }

    lwip_init();

    ipaddr = cfg.ip;
    netmask = cfg.netmask;
    gateway = cfg.gateway;

    if (mipos_lwip_netif_open(&iface,
                              cfg.mac,
                              &ipaddr,
                              &netmask,
                              &gateway,
                              tap_link_output,
                              &app) != ERR_OK) {
        fprintf(stderr, "mipos_lwip_netif_open failed\n");
        set_tap_media_status(app.tap, 0);
        CloseHandle(app.tap);
        return 1;
    }
    app.iface = &iface;
    app.ping_pcb = raw_new(IP_PROTO_ICMP);
    if (!app.ping_pcb) {
        fprintf(stderr, "raw_new(ICMP) failed\n");
        set_tap_media_status(app.tap, 0);
        CloseHandle(app.tap);
        return 1;
    }
    raw_recv(app.ping_pcb, ping_recv, &app);

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    printf("tap: %s\n", tap_path);
    printf("mipOS MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           cfg.mac[0], cfg.mac[1], cfg.mac[2],
           cfg.mac[3], cfg.mac[4], cfg.mac[5]);
    print_ip("mipOS IP: ", &cfg.ip);
    printf("\n");
    printf("Ready. Ping the mipOS IP from Windows. Press Ctrl-C to stop.\n");
    if (cfg.verbose) {
        printf("Packet log enabled. Type 'quiet' to suppress RX/TX lines.\n");
    } else {
        printf("Packet log disabled. Type 'verbose' to show RX/TX lines.\n");
    }
    printf("Type 'help' for mipOS network commands.\n");
    print_prompt();

    while (!g_stop) {
        unsigned int frame_len = 0;
        int rc = tap_read_frame(app.tap, frame, sizeof(frame), &frame_len);

        poll_console(&app, &cfg);
        mipos_lwip_netif_poll(&iface);

        if (rc == 0) {
            continue;
        }
        if (rc < 0) {
            fprintf(stderr, "TAP read failed: %lu\n", GetLastError());
            break;
        }
        if (frame_len > 65535 || is_own_ethernet_frame(cfg.mac, frame, frame_len)) {
            continue;
        }

        ++app.rx_count;
        if (app.verbose) {
            log_frame("RX", app.ip, frame, frame_len);
        }

        mipos_lwip_netif_input(&iface, frame, (uint16_t)frame_len);
    }

    printf("Stopped. RX=%u TX=%u\n", app.rx_count, app.tx_count);
    dhcp_stop(&iface.netif);
    if (app.tcp_pcb) {
        tcp_abort(app.tcp_pcb);
    }
    raw_remove(app.ping_pcb);
    set_tap_media_status(app.tap, 0);
    CloseHandle(app.tap);

    return 0;
}
