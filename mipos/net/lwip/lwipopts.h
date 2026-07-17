#ifndef MIPOS_LWIPOPTS_H
#define MIPOS_LWIPOPTS_H

#define NO_SYS                          1
#define SYS_LIGHTWEIGHT_PROT            0
#define LWIP_TIMERS                     1

#define LWIP_IPV4                       1
#define LWIP_IPV6                       0
#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_ICMP                       1
#define LWIP_RAW                        1
#define LWIP_UDP                        1
#define LWIP_TCP                        1

#define LWIP_DHCP                       1
#define LWIP_DHCP_DOES_ACD_CHECK        0
#define LWIP_AUTOIP                     0
#define LWIP_DNS                        1
#define DNS_TABLE_SIZE                  4
#define DNS_MAX_SERVERS                 2
#define DNS_MAX_RETRIES                 2
#define LWIP_DNS_SECURE                 0
#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0

#define LWIP_NETIF_HOSTNAME             1
#define LWIP_NETIF_STATUS_CALLBACK      0
#define LWIP_NETIF_LINK_CALLBACK        0
#define LWIP_NETIF_EXT_STATUS_CALLBACK  0
#define LWIP_NETIF_TX_SINGLE_PBUF       1
#define LWIP_NETIF_LOOPBACK             0

#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        (24 * 1024)
#define MEMP_NUM_PBUF                   16
#define MEMP_NUM_NETBUF                 0
#define MEMP_NUM_NETCONN                0
#define MEMP_NUM_TCP_PCB                4
#define MEMP_NUM_TCP_PCB_LISTEN         4
#define MEMP_NUM_TCP_SEG                16
#define MEMP_NUM_UDP_PCB                8
#define MEMP_NUM_SYS_TIMEOUT            8
#define PBUF_POOL_SIZE                  16
#define PBUF_POOL_BUFSIZE               1536

#define TCP_MSS                         536
#define TCP_WND                         (4 * TCP_MSS)
#define TCP_SND_BUF                     (4 * TCP_MSS)
#define TCP_SND_QUEUELEN                8
#define TCP_QUEUE_OOSEQ                 0

#define IP_FORWARD                      0
#define IP_REASSEMBLY                   0
#define IP_FRAG                         0
#define LWIP_BROADCAST_PING             0
#define LWIP_MULTICAST_PING             0

#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_ICMP               1
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_ICMP             1
#define CHECKSUM_CHECK_TCP              1
#define CHECKSUM_CHECK_UDP              1

#define LWIP_STATS                      0
#define LWIP_DEBUG                      0

#endif
