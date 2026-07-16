#ifndef MIPOS_LWIP_NETIF_H
#define MIPOS_LWIP_NETIF_H

#include <stddef.h>
#include <stdint.h>

#include "lwip/err.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef err_t (*mipos_lwip_link_output_fn)(void* ctx,
                                           const uint8_t* frame,
                                           uint16_t frame_len);

typedef struct mipos_lwip_netif {
    struct netif netif;
    uint8_t hwaddr[6];
    mipos_lwip_link_output_fn link_output;
    void* link_output_ctx;
} mipos_lwip_netif_t;

err_t mipos_lwip_netif_open(mipos_lwip_netif_t* iface,
                            const uint8_t hwaddr[6],
                            const ip4_addr_t* ipaddr,
                            const ip4_addr_t* netmask,
                            const ip4_addr_t* gateway,
                            mipos_lwip_link_output_fn link_output,
                            void* link_output_ctx);

err_t mipos_lwip_netif_input(mipos_lwip_netif_t* iface,
                             const uint8_t* frame,
                             uint16_t frame_len);

void mipos_lwip_netif_poll(mipos_lwip_netif_t* iface);

#ifdef __cplusplus
}
#endif

#endif
