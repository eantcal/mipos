/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

// lwIP ARP/ICMP self-test for QEMU lm3s6965evb.
//
// This validates the mipOS lwIP netif adapter in the cross-compiled QEMU
// firmware environment before a real Stellaris Ethernet MMIO driver is added.

#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "mipos_bsp.h"
#include "mipos_lwip_netif.h"

#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------------- */

static const uint8_t k_mipos_mac[6] = {0x02, 0, 0, 0, 0, 1};
static const uint8_t k_host_mac[6] = {0x02, 0, 0, 0, 0, 2};
static const uint8_t k_mipos_ip[4] = {192, 168, 1, 2};
static const uint8_t k_host_ip[4] = {192, 168, 1, 10};

static uint8_t g_frame[1536];
static uint16_t g_frame_len;
static unsigned int g_frame_count;

/* ------------------------------------------------------------------------- */

static void uart_puts(const char* s)
{
    while (*s) {
        if (*s == '\n') {
            mipos_bsp_rs232_putc('\r');
        }
        mipos_bsp_rs232_putc((unsigned char)*s++);
    }
}

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

static err_t capture_output(void* ctx, const uint8_t* frame, uint16_t frame_len)
{
    (void)ctx;

    if (frame_len > sizeof(g_frame)) {
        return ERR_IF;
    }

    memcpy(g_frame, frame, frame_len);
    g_frame_len = frame_len;
    ++g_frame_count;

    return ERR_OK;
}

static void clear_capture(void)
{
    memset(g_frame, 0, sizeof(g_frame));
    g_frame_len = 0;
    g_frame_count = 0;
}

static void make_arp_request(uint8_t* frame)
{
    memset(frame, 0, 42);
    memset(frame, 0xff, 6);
    memcpy(frame + 6, k_host_mac, 6);
    write_be16(frame, 12, 0x0806);

    write_be16(frame, 14, 1);
    write_be16(frame, 16, 0x0800);
    frame[18] = 6;
    frame[19] = 4;
    write_be16(frame, 20, 1);
    memcpy(frame + 22, k_host_mac, 6);
    memcpy(frame + 28, k_host_ip, 4);
    memcpy(frame + 38, k_mipos_ip, 4);
}

static void make_icmp_echo_request(uint8_t* frame)
{
    memset(frame, 0, 42);
    memcpy(frame, k_mipos_mac, 6);
    memcpy(frame + 6, k_host_mac, 6);
    write_be16(frame, 12, 0x0800);

    frame[14] = 0x45;
    write_be16(frame, 16, 28);
    write_be16(frame, 18, 0x1234);
    frame[22] = 64;
    frame[23] = 1;
    memcpy(frame + 26, k_host_ip, 4);
    memcpy(frame + 30, k_mipos_ip, 4);
    write_be16(frame, 24, checksum(frame + 14, 20));

    frame[34] = 8;
    write_be16(frame, 38, 0x4567);
    write_be16(frame, 40, 1);
    write_be16(frame, 36, checksum(frame + 34, 8));
}

static int check_arp_reply(void)
{
    return g_frame_count == 1 &&
           g_frame_len >= 42 &&
           memcmp(g_frame, k_host_mac, 6) == 0 &&
           memcmp(g_frame + 6, k_mipos_mac, 6) == 0 &&
           read_be16(g_frame + 12) == 0x0806 &&
           read_be16(g_frame + 20) == 2 &&
           memcmp(g_frame + 22, k_mipos_mac, 6) == 0 &&
           memcmp(g_frame + 28, k_mipos_ip, 4) == 0;
}

static int check_icmp_reply(void)
{
    return g_frame_count == 1 &&
           g_frame_len >= 42 &&
           memcmp(g_frame, k_host_mac, 6) == 0 &&
           memcmp(g_frame + 6, k_mipos_mac, 6) == 0 &&
           read_be16(g_frame + 12) == 0x0800 &&
           g_frame[23] == 1 &&
           memcmp(g_frame + 26, k_mipos_ip, 4) == 0 &&
           memcmp(g_frame + 30, k_host_ip, 4) == 0 &&
           g_frame[34] == 0 &&
           read_be16(g_frame + 38) == 0x4567 &&
           read_be16(g_frame + 40) == 1;
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    mipos_lwip_netif_t iface;
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gateway;
    uint8_t input[64];

    mipos_bsp_configure_rs232(115200);
    uart_puts("mipOS QEMU lwIP ARP/ICMP self-test\n");

    lwip_init();

    IP4_ADDR(&ipaddr, 192, 168, 1, 2);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gateway, 192, 168, 1, 1);

    if (mipos_lwip_netif_open(&iface,
                              k_mipos_mac,
                              &ipaddr,
                              &netmask,
                              &gateway,
                              capture_output,
                              0) != ERR_OK) {
        uart_puts("FAIL: netif open\n");
        for (;;) {
        }
    }

    clear_capture();
    make_arp_request(input);
    if (mipos_lwip_netif_input(&iface, input, 42) != ERR_OK ||
        !check_arp_reply()) {
        uart_puts("FAIL: ARP reply\n");
        for (;;) {
        }
    }
    uart_puts("ARP reply OK\n");

    clear_capture();
    make_icmp_echo_request(input);
    if (mipos_lwip_netif_input(&iface, input, 42) != ERR_OK ||
        !check_icmp_reply()) {
        uart_puts("FAIL: ICMP echo reply\n");
        for (;;) {
        }
    }
    uart_puts("ICMP echo reply OK\n");
    uart_puts("PASS\n");

    for (;;) {
    }

    return 0;
}

