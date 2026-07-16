#include "mipos_lwip_netif.h"

#include <string.h>

#include "lwip/etharp.h"
#include "lwip/init.h"
#include "lwip/pbuf.h"
#include "netif/ethernet.h"

static err_t mipos_lwip_low_level_output(struct netif* netif, struct pbuf* p)
{
    mipos_lwip_netif_t* iface = (mipos_lwip_netif_t*)netif->state;
    uint8_t frame[1536];
    uint16_t copied;

    if (!iface || !iface->link_output || p->tot_len > sizeof(frame)) {
        return ERR_IF;
    }

    copied = pbuf_copy_partial(p, frame, p->tot_len, 0);
    if (copied != p->tot_len) {
        return ERR_IF;
    }

    return iface->link_output(iface->link_output_ctx, frame, copied);
}

static err_t mipos_lwip_low_level_init(struct netif* netif)
{
    mipos_lwip_netif_t* iface = (mipos_lwip_netif_t*)netif->state;

    netif->name[0] = 'm';
    netif->name[1] = 'p';
    netif->output = etharp_output;
    netif->linkoutput = mipos_lwip_low_level_output;
    netif->mtu = 1500;
    netif->hwaddr_len = 6;
    memcpy(netif->hwaddr, iface->hwaddr, 6);
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    return ERR_OK;
}

err_t mipos_lwip_netif_open(mipos_lwip_netif_t* iface,
                            const uint8_t hwaddr[6],
                            const ip4_addr_t* ipaddr,
                            const ip4_addr_t* netmask,
                            const ip4_addr_t* gateway,
                            mipos_lwip_link_output_fn link_output,
                            void* link_output_ctx)
{
    struct netif* added;

    if (!iface || !hwaddr || !ipaddr || !netmask || !gateway || !link_output) {
        return ERR_ARG;
    }

    memset(iface, 0, sizeof(*iface));
    memcpy(iface->hwaddr, hwaddr, sizeof(iface->hwaddr));
    iface->link_output = link_output;
    iface->link_output_ctx = link_output_ctx;

    added = netif_add(&iface->netif,
                      ipaddr,
                      netmask,
                      gateway,
                      iface,
                      mipos_lwip_low_level_init,
                      ethernet_input);
    if (!added) {
        return ERR_IF;
    }

    netif_set_default(&iface->netif);
    netif_set_up(&iface->netif);
    netif_set_link_up(&iface->netif);

    return ERR_OK;
}

err_t mipos_lwip_netif_input(mipos_lwip_netif_t* iface,
                             const uint8_t* frame,
                             uint16_t frame_len)
{
    struct pbuf* p;
    err_t err;

    if (!iface || !frame || frame_len == 0) {
        return ERR_ARG;
    }

    p = pbuf_alloc(PBUF_RAW, frame_len, PBUF_POOL);
    if (!p) {
        return ERR_MEM;
    }

    if (pbuf_take(p, frame, frame_len) != ERR_OK) {
        pbuf_free(p);
        return ERR_MEM;
    }

    err = iface->netif.input(p, &iface->netif);
    if (err != ERR_OK) {
        pbuf_free(p);
    }

    return err;
}

void mipos_lwip_netif_poll(mipos_lwip_netif_t* iface)
{
    (void)iface;
}
