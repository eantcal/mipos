/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/*
 * Interactive lwIP console for the QEMU lm3s6965evb firmware.
 *
 * This first QEMU networking console uses an in-memory Ethernet peer. It
 * exercises the same lwIP netif used by the host TAP/pcap examples while a
 * future Stellaris Ethernet MMIO driver can replace only the link backend.
 */

#include "lwip/etharp.h"
#include "lwip/inet_chksum.h"
#include "lwip/init.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/prot/icmp.h"
#include "lwip/raw.h"
#include "mipos.h"
#include "mipos_bsp.h"
#include "mipos_lwip_netif.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */

#define NET_FRAME_MAX 1536u
#define NET_PENDING_MAX 4u

static const uint8_t k_mipos_mac[6] = {0x02, 0, 0, 0, 0, 1};
static const uint8_t k_peer_mac[6] = {0x02, 0, 0, 0, 0, 2};

static char root_stack[4096];
static char net_task_stack[4096];

typedef struct pending_frame {
    uint8_t data[NET_FRAME_MAX];
    uint16_t len;
} pending_frame_t;

typedef struct net_app {
    mipos_lwip_netif_t iface;
    struct raw_pcb* ping_pcb;
    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gateway;
    ip4_addr_t peer_ip;
    pending_frame_t pending[NET_PENDING_MAX];
    unsigned int pending_count;
    unsigned int rx_count;
    unsigned int tx_count;
    unsigned int dropped_count;
    uint16_t ping_id;
    uint16_t ping_seq;
} net_app_t;

static net_app_t g_net;

/* ------------------------------------------------------------------------- */

static uint16_t read_be16(const uint8_t* data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static void write_be16(uint8_t* frame, unsigned int off, uint16_t value)
{
    frame[off] = (uint8_t)(value >> 8);
    frame[off + 1] = (uint8_t)(value & 0xffu);
}

static uint16_t checksum(const uint8_t* data, unsigned int len)
{
    uint32_t sum = 0;

    while (len > 1) {
        sum += read_be16(data);
        data += 2;
        len -= 2;
    }

    if (len) {
        sum += (uint16_t)((uint16_t)data[0] << 8);
    }

    while (sum >> 16) {
        sum = (sum & 0xffffu) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}

static void print_ip4(const char* label, const ip4_addr_t* ip)
{
    char buf[64];

    sprintf(buf,
            "%s%u.%u.%u.%u\r\n",
            label,
            ip4_addr1(ip),
            ip4_addr2(ip),
            ip4_addr3(ip),
            ip4_addr4(ip));
    mipos_puts(buf);
}

static void print_mac(const char* label, const uint8_t mac[6])
{
    char buf[64];

    sprintf(buf,
            "%s%02x:%02x:%02x:%02x:%02x:%02x\r\n",
            label,
            mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]);
    mipos_puts(buf);
}

static int parse_ip4(const char* text, ip4_addr_t* out)
{
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;

    if (sscanf(text, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        return 0;
    }

    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return 0;
    }

    IP4_ADDR(out, a, b, c, d);
    return 1;
}

static int ip4_equals_bytes(const ip4_addr_t* ip, const uint8_t bytes[4])
{
    return ip4_addr1(ip) == bytes[0] && ip4_addr2(ip) == bytes[1] &&
           ip4_addr3(ip) == bytes[2] && ip4_addr4(ip) == bytes[3];
}

static void queue_frame(net_app_t* app, const uint8_t* data, uint16_t len)
{
    if (len > NET_FRAME_MAX || app->pending_count >= NET_PENDING_MAX) {
        ++app->dropped_count;
        return;
    }

    memcpy(app->pending[app->pending_count].data, data, len);
    app->pending[app->pending_count].len = len;
    ++app->pending_count;
}

static void make_arp_reply(net_app_t* app, const uint8_t* request)
{
    uint8_t reply[42];
    uint8_t peer_ip[4];

    peer_ip[0] = ip4_addr1(&app->peer_ip);
    peer_ip[1] = ip4_addr2(&app->peer_ip);
    peer_ip[2] = ip4_addr3(&app->peer_ip);
    peer_ip[3] = ip4_addr4(&app->peer_ip);

    memset(reply, 0, sizeof(reply));
    memcpy(reply, k_mipos_mac, 6);
    memcpy(reply + 6, k_peer_mac, 6);
    write_be16(reply, 12, 0x0806);

    write_be16(reply, 14, 1);
    write_be16(reply, 16, 0x0800);
    reply[18] = 6;
    reply[19] = 4;
    write_be16(reply, 20, 2);
    memcpy(reply + 22, k_peer_mac, 6);
    memcpy(reply + 28, peer_ip, 4);
    memcpy(reply + 32, request + 22, 6);
    memcpy(reply + 38, request + 28, 4);

    queue_frame(app, reply, sizeof(reply));
}

static void make_icmp_echo_reply(net_app_t* app,
                                 const uint8_t* request,
                                 uint16_t len)
{
    uint8_t reply[NET_FRAME_MAX];
    unsigned int ip;
    unsigned int ihl;
    unsigned int total_len;
    unsigned int icmp;

    if (len > sizeof(reply) || len < 42) {
        return;
    }

    memcpy(reply, request, len);
    memcpy(reply, k_mipos_mac, 6);
    memcpy(reply + 6, k_peer_mac, 6);

    ip = 14;
    ihl = (unsigned int)(reply[ip] & 0x0fu) * 4u;
    if (ihl < 20u || len < ip + ihl + 8u) {
        return;
    }

    total_len = read_be16(reply + ip + 2);
    if (total_len < ihl + 8u || ip + total_len > len) {
        return;
    }

    memcpy(reply + ip + 12, request + ip + 16, 4);
    memcpy(reply + ip + 16, request + ip + 12, 4);
    write_be16(reply, ip + 10, 0);
    write_be16(reply, ip + 10, checksum(reply + ip, ihl));

    icmp = ip + ihl;
    reply[icmp] = 0;
    reply[icmp + 1] = 0;
    write_be16(reply, icmp + 2, 0);
    write_be16(reply, icmp + 2, checksum(reply + icmp, total_len - ihl));

    queue_frame(app, reply, len);
}

static int frame_targets_peer(net_app_t* app, const uint8_t ip[4])
{
    return ip4_equals_bytes(&app->peer_ip, ip) ||
           ip4_equals_bytes(&app->gateway, ip);
}

static err_t peer_link_output(void* ctx, const uint8_t* frame, uint16_t len)
{
    net_app_t* app = (net_app_t*)ctx;

    ++app->tx_count;

    if (len >= 42 && read_be16(frame + 12) == 0x0806 &&
        read_be16(frame + 20) == 1 && frame_targets_peer(app, frame + 38)) {
        make_arp_reply(app, frame);
    } else if (len >= 42 && read_be16(frame + 12) == 0x0800 &&
               frame[23] == 1 && frame[34] == 8 &&
               frame_targets_peer(app, frame + 30)) {
        make_icmp_echo_reply(app, frame, len);
    }

    return ERR_OK;
}

static void report_icmp_echo_reply(net_app_t* app,
                                   const uint8_t* frame,
                                   uint16_t len)
{
    unsigned int ip;
    unsigned int ihl;
    unsigned int icmp;
    uint16_t id;
    uint16_t seq;
    char buf[96];

    if (len < 42 || read_be16(frame + 12) != 0x0800 || frame[23] != 1) {
        return;
    }

    ip = 14;
    ihl = (unsigned int)(frame[ip] & 0x0fu) * 4u;
    if (ihl < 20u || len < ip + ihl + 8u) {
        return;
    }

    icmp = ip + ihl;
    if (frame[icmp] != ICMP_ER || !ip4_equals_bytes(&app->ip, frame + ip + 16)) {
        return;
    }

    id = read_be16(frame + icmp + 4);
    seq = read_be16(frame + icmp + 6);
    if (id != app->ping_id) {
        return;
    }

    sprintf(buf,
            "ping reply from %u.%u.%u.%u seq=%u bytes=%u\r\n",
            frame[ip + 12],
            frame[ip + 13],
            frame[ip + 14],
            frame[ip + 15],
            (unsigned int)seq,
            (unsigned int)(read_be16(frame + ip + 2) - ihl));
    mipos_puts(buf);
    mipos_puts("mipOS-net>");
}

static u8_t ping_recv(void* arg,
                      struct raw_pcb* pcb,
                      struct pbuf* p,
                      const ip_addr_t* addr)
{
    net_app_t* app = (net_app_t*)arg;
    uint8_t header[8];
    uint16_t id;
    uint16_t seq;
    const ip4_addr_t* ip4;
    char buf[96];

    (void)pcb;

    if (!p || p->tot_len < sizeof(header) || !IP_IS_V4(addr)) {
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

    ip4 = ip_2_ip4(addr);
    sprintf(buf,
            "ping reply from %u.%u.%u.%u seq=%u bytes=%u\r\n",
            ip4_addr1(ip4),
            ip4_addr2(ip4),
            ip4_addr3(ip4),
            ip4_addr4(ip4),
            (unsigned int)seq,
            (unsigned int)p->tot_len);
    mipos_puts(buf);
    mipos_puts("mipOS-net>");

    pbuf_free(p);
    return 1;
}

static void send_ping(net_app_t* app, const ip4_addr_t* dst)
{
    struct pbuf* p;
    struct icmp_echo_hdr* echo;
    ip_addr_t dst_addr;
    const unsigned int payload_len = 16;
    uint8_t* payload;
    unsigned int i;
    err_t err;
    char buf[64];

    p = pbuf_alloc(PBUF_IP, (u16_t)(sizeof(*echo) + payload_len), PBUF_RAM);
    if (!p) {
        mipos_puts("ping: pbuf_alloc failed\r\n");
        return;
    }

    echo = (struct icmp_echo_hdr*)p->payload;
    ICMPH_TYPE_SET(echo, ICMP_ECHO);
    ICMPH_CODE_SET(echo, 0);
    echo->id = lwip_htons(app->ping_id);
    echo->seqno = lwip_htons(++app->ping_seq);

    payload = (uint8_t*)p->payload + sizeof(*echo);
    for (i = 0; i < payload_len; ++i) {
        payload[i] = (uint8_t)('a' + (i % 26u));
    }

    echo->chksum = 0;
    echo->chksum = inet_chksum(echo, p->len);

    sprintf(buf,
            "ping %u.%u.%u.%u seq=%u\r\n",
            ip4_addr1(dst),
            ip4_addr2(dst),
            ip4_addr3(dst),
            ip4_addr4(dst),
            (unsigned int)app->ping_seq);
    mipos_puts(buf);

    ip_addr_copy_from_ip4(dst_addr, *dst);
    err = raw_sendto(app->ping_pcb, p, &dst_addr);
    if (err != ERR_OK) {
        sprintf(buf, "ping: raw_sendto failed: %d\r\n", (int)err);
        mipos_puts(buf);
    }

    pbuf_free(p);
}

/* ------------------------------------------------------------------------- */

static int cmd_ip(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    print_ip4("mipOS IP: ", &g_net.ip);
    print_ip4("netmask:  ", &g_net.netmask);
    print_ip4("gateway:  ", &g_net.gateway);
    print_mac("mipOS MAC: ", k_mipos_mac);

    return 0;
}

static int cmd_peer(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    print_ip4("peer IP: ", &g_net.peer_ip);
    print_mac("peer MAC: ", k_peer_mac);
    mipos_puts("backend: in-memory QEMU Ethernet peer\r\n");

    return 0;
}

static int cmd_route(int argc, char* argv[])
{
    ip4_addr_t dst;
    ip4_addr_t network;
    char buf[96];

    if (argc > 1) {
        if (!parse_ip4(argv[1], &dst)) {
            mipos_puts("usage: route [ip]\r\n");
            return -1;
        }

        if (ip4_addr_netcmp(&dst, &g_net.ip, &g_net.netmask)) {
            mipos_puts("route: connected via mp0\r\n");
        } else {
            print_ip4("route: via gateway ", &g_net.gateway);
        }
        return 0;
    }

    network.addr = g_net.ip.addr & g_net.netmask.addr;
    mipos_puts("destination      gateway          netmask          iface  type\r\n");
    sprintf(buf,
            "%u.%u.%u.%u      0.0.0.0          %u.%u.%u.%u  mp0    connected\r\n",
            ip4_addr1(&network),
            ip4_addr2(&network),
            ip4_addr3(&network),
            ip4_addr4(&network),
            ip4_addr1(&g_net.netmask),
            ip4_addr2(&g_net.netmask),
            ip4_addr3(&g_net.netmask),
            ip4_addr4(&g_net.netmask));
    mipos_puts(buf);
    sprintf(buf,
            "0.0.0.0          %u.%u.%u.%u     0.0.0.0          mp0    default\r\n",
            ip4_addr1(&g_net.gateway),
            ip4_addr2(&g_net.gateway),
            ip4_addr3(&g_net.gateway),
            ip4_addr4(&g_net.gateway));
    mipos_puts(buf);

    return 0;
}

static int cmd_arp(int argc, char* argv[])
{
    ip4_addr_t ip;
    const ip4_addr_t* found_ip = 0;
    struct eth_addr* eth = 0;
    int index;
    char buf[96];

    if (argc > 1) {
        if (!parse_ip4(argv[1], &ip)) {
            mipos_puts("usage: arp [ip]\r\n");
            return -1;
        }
    } else {
        ip = g_net.peer_ip;
    }

    index = etharp_find_addr(&g_net.iface.netif, &ip, &eth, &found_ip);
    if (index >= 0 && eth && found_ip) {
        sprintf(buf,
                "arp %u.%u.%u.%u -> %02x:%02x:%02x:%02x:%02x:%02x\r\n",
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
        mipos_puts(buf);
    } else {
        sprintf(buf,
                "arp %u.%u.%u.%u unresolved; sending ARP request\r\n",
                ip4_addr1(&ip),
                ip4_addr2(&ip),
                ip4_addr3(&ip),
                ip4_addr4(&ip));
        mipos_puts(buf);
        etharp_request(&g_net.iface.netif, &ip);
    }

    return 0;
}

static int cmd_ping(int argc, char* argv[])
{
    ip4_addr_t dst;

    if (argc > 1) {
        if (!parse_ip4(argv[1], &dst)) {
            mipos_puts("usage: ping <ip>\r\n");
            return -1;
        }
    } else {
        dst = g_net.peer_ip;
    }

    send_ping(&g_net, &dst);
    return 0;
}

static int cmd_stats(int argc, char* argv[])
{
    char buf[80];

    (void)argc;
    (void)argv;

    sprintf(buf,
            "RX=%u TX=%u dropped=%u pending=%u\r\n",
            g_net.rx_count,
            g_net.tx_count,
            g_net.dropped_count,
            g_net.pending_count);
    mipos_puts(buf);

    return 0;
}

static mipos_console_cmd_t net_cmd_list[] = {
    { "ip", cmd_ip, "ip - show IPv4 configuration" },
    { "route", cmd_route, "route [ip] - show or resolve IPv4 route" },
    { "arp", cmd_arp, "arp [ip] - resolve/show ARP entry" },
    { "ping", cmd_ping, "ping [ip] - send ICMP echo request" },
    { "peer", cmd_peer, "peer - show QEMU in-memory peer" },
    { "stats", cmd_stats, "stats - show network counters" },

    { 0, 0, 0 }
};

/* ------------------------------------------------------------------------- */

static int net_task(task_param_t param)
{
    net_app_t* app = (net_app_t*)param;

    while (1) {
        while (app->pending_count > 0) {
            pending_frame_t frame;
            unsigned int remaining;
            unsigned int i;

            frame = app->pending[0];
            remaining = app->pending_count - 1u;
            for (i = 0; i < remaining; ++i) {
                app->pending[i] = app->pending[i + 1u];
            }
            app->pending_count = remaining;

            ++app->rx_count;
            report_icmp_echo_reply(app, frame.data, frame.len);
            mipos_lwip_netif_input(&app->iface, frame.data, frame.len);
        }

        mipos_lwip_netif_poll(&app->iface);
        mipos_tm_msleep(10);
    }

    return 0;
}

static int root_task(task_param_t param)
{
    mipos_console_init_t cinit = {0};

    (void)param;

    memset(&g_net, 0, sizeof(g_net));
    g_net.ping_id = 0x4d50;

    IP4_ADDR(&g_net.ip, 192, 168, 1, 2);
    IP4_ADDR(&g_net.netmask, 255, 255, 255, 0);
    IP4_ADDR(&g_net.gateway, 192, 168, 1, 10);
    IP4_ADDR(&g_net.peer_ip, 192, 168, 1, 10);

    lwip_init();
    if (mipos_lwip_netif_open(&g_net.iface,
                              k_mipos_mac,
                              &g_net.ip,
                              &g_net.netmask,
                              &g_net.gateway,
                              peer_link_output,
                              &g_net) != ERR_OK) {
        mipos_puts("mipos_lwip_netif_open failed\r\n");
        mipos_t_suspend();
        return -1;
    }

    g_net.ping_pcb = raw_new(IP_PROTO_ICMP);
    if (!g_net.ping_pcb) {
        mipos_puts("raw_new(ICMP) failed\r\n");
        mipos_t_suspend();
        return -1;
    }
    raw_recv(g_net.ping_pcb, ping_recv, &g_net);

    mipos_t_create("net",
                   net_task,
                   (task_param_t)&g_net,
                   net_task_stack,
                   sizeof(net_task_stack),
                   MIPOS_NO_RT);

    cinit.prompt = "\nmipOS-net>";
    cinit.end_line_char = '\r';
    cinit.flags = CONSOLE_FLG_ECHO | CONSOLE_TX_CRLF;

    mipos_console_register_cmd_list(net_cmd_list);
    mipos_console_init(&cinit);

    mipos_puts("mipOS QEMU lwIP network console\r\n");
    mipos_puts("Type 'ip', 'peer', 'arp', and 'ping'.\r\n");
    print_ip4("mipOS IP: ", &g_net.ip);
    print_ip4("peer IP: ", &g_net.peer_ip);

    mipos_t_suspend();
    return 0;
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    mipos_bsp_configure_rs232(115200);
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));

    for (;;) {
    }

    return 0;
}
