#ifndef MIPOS_LWIPOPTS_H
#define MIPOS_LWIPOPTS_H

#define NO_SYS                          1
#define SYS_LIGHTWEIGHT_PROT            0
#define LWIP_TIMERS                     0

#define LWIP_IPV4                       1
#define LWIP_IPV6                       0
#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_ICMP                       1
#define LWIP_RAW                        0
#define LWIP_UDP                        0
#define LWIP_TCP                        0

#define LWIP_DHCP                       0
#define LWIP_AUTOIP                     0
#define LWIP_DNS                        0
#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0

#define LWIP_NETIF_HOSTNAME             0
#define LWIP_NETIF_STATUS_CALLBACK      0
#define LWIP_NETIF_LINK_CALLBACK        0
#define LWIP_NETIF_EXT_STATUS_CALLBACK  0
#define LWIP_NETIF_TX_SINGLE_PBUF       1
#define LWIP_NETIF_LOOPBACK             0

#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        (8 * 1024)
#define MEMP_NUM_PBUF                   8
#define MEMP_NUM_NETBUF                 0
#define MEMP_NUM_NETCONN                0
#define MEMP_NUM_TCP_PCB                0
#define MEMP_NUM_TCP_PCB_LISTEN         0
#define MEMP_NUM_TCP_SEG                0
#define MEMP_NUM_UDP_PCB                0
#define PBUF_POOL_SIZE                  8
#define PBUF_POOL_BUFSIZE               1536

#define IP_FORWARD                      0
#define IP_REASSEMBLY                   0
#define IP_FRAG                         0
#define LWIP_BROADCAST_PING             0
#define LWIP_MULTICAST_PING             0

#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_ICMP               1
#define CHECKSUM_GEN_TCP                0
#define CHECKSUM_GEN_UDP                0
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_ICMP             1
#define CHECKSUM_CHECK_TCP              0
#define CHECKSUM_CHECK_UDP              0

#define LWIP_STATS                      0
#define LWIP_DEBUG                      0

#endif
