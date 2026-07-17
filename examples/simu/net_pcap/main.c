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
#include <windows.h>
#else
#error "example-net-pcap currently supports Windows/Npcap only"
#endif

#define PCAP_ERRBUF_SIZE 256

typedef struct pcap pcap_t;

typedef struct pcap_if {
    struct pcap_if* next;
    char* name;
    char* description;
    void* addresses;
    unsigned int flags;
} pcap_if_t;

struct pcap_timeval {
    long tv_sec;
    long tv_usec;
};

struct pcap_pkthdr {
    struct pcap_timeval ts;
    unsigned int caplen;
    unsigned int len;
};

typedef int (*pcap_findalldevs_fn)(pcap_if_t** alldevs, char* errbuf);
typedef void (*pcap_freealldevs_fn)(pcap_if_t* alldevs);
typedef pcap_t* (*pcap_open_live_fn)(const char* source,
                                     int snaplen,
                                     int promisc,
                                     int to_ms,
                                     char* errbuf);
typedef int (*pcap_next_ex_fn)(pcap_t* p,
                               struct pcap_pkthdr** pkt_header,
                               const unsigned char** pkt_data);
typedef int (*pcap_sendpacket_fn)(pcap_t* p,
                                  const unsigned char* buf,
                                  int size);
typedef char* (*pcap_geterr_fn)(pcap_t* p);
typedef void (*pcap_close_fn)(pcap_t* p);
typedef const char* (*pcap_lib_version_fn)(void);

typedef struct pcap_api {
    HMODULE dll;
    pcap_findalldevs_fn findalldevs;
    pcap_freealldevs_fn freealldevs;
    pcap_open_live_fn open_live;
    pcap_next_ex_fn next_ex;
    pcap_sendpacket_fn sendpacket;
    pcap_geterr_fn geterr;
    pcap_close_fn close;
    pcap_lib_version_fn lib_version;
} pcap_api_t;

typedef struct app_config {
    const char* adapter;
    int adapter_index;
    int verbose;
    uint8_t mac[6];
    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gateway;
} app_config_t;

typedef struct app_context {
    pcap_api_t* pcap;
    pcap_t* handle;
    uint8_t mac[6];
    uint8_t ip[4];
    int verbose;
    unsigned int rx_count;
    unsigned int tx_count;
} app_context_t;

static volatile int g_stop = 0;

static void on_signal(int sig)
{
    (void)sig;
    g_stop = 1;
}

static FARPROC load_symbol(HMODULE dll, const char* name)
{
    FARPROC proc = GetProcAddress(dll, name);

    if (!proc) {
        fprintf(stderr, "missing Npcap symbol: %s\n", name);
    }

    return proc;
}

static int load_pcap(pcap_api_t* api)
{
    memset(api, 0, sizeof(*api));

    api->dll = LoadLibraryA("wpcap.dll");
    if (!api->dll) {
        fprintf(stderr,
                "wpcap.dll not found. Install Npcap from https://npcap.com/\n");
        return 0;
    }

#define LOAD_PCAP_SYMBOL(field, type, name)                                \
    do {                                                                   \
        api->field = (type)load_symbol(api->dll, name);                    \
        if (!api->field) {                                                 \
            FreeLibrary(api->dll);                                         \
            memset(api, 0, sizeof(*api));                                  \
            return 0;                                                      \
        }                                                                  \
    } while (0)

    LOAD_PCAP_SYMBOL(findalldevs, pcap_findalldevs_fn, "pcap_findalldevs");
    LOAD_PCAP_SYMBOL(freealldevs, pcap_freealldevs_fn, "pcap_freealldevs");
    LOAD_PCAP_SYMBOL(open_live, pcap_open_live_fn, "pcap_open_live");
    LOAD_PCAP_SYMBOL(next_ex, pcap_next_ex_fn, "pcap_next_ex");
    LOAD_PCAP_SYMBOL(sendpacket, pcap_sendpacket_fn, "pcap_sendpacket");
    LOAD_PCAP_SYMBOL(geterr, pcap_geterr_fn, "pcap_geterr");
    LOAD_PCAP_SYMBOL(close, pcap_close_fn, "pcap_close");
    LOAD_PCAP_SYMBOL(lib_version, pcap_lib_version_fn, "pcap_lib_version");

#undef LOAD_PCAP_SYMBOL

    return 1;
}

static void unload_pcap(pcap_api_t* api)
{
    if (api->dll) {
        FreeLibrary(api->dll);
    }

    memset(api, 0, sizeof(*api));
}

static void usage(const char* exe)
{
    printf("Usage:\n");
    printf("  %s --list\n", exe);
    printf("  %s --adapter <index-or-name> [--ip 10.77.0.2] "
           "[--netmask 255.255.255.0] [--gateway 10.77.0.1] [--quiet]\n",
           exe);
    printf("\n");
    printf("For ARP/ping testing, bind this to a Layer-2 virtual adapter "
           "(for example TAP),\n");
    printf("assign the host side an address in the same subnet, then ping "
           "the mipOS IP.\n");
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

static int parse_int(const char* text, int* out)
{
    char* end = NULL;
    long value;

    if (!text || !isdigit((unsigned char)text[0])) {
        return 0;
    }

    value = strtol(text, &end, 10);
    if (!end || *end != 0 || value <= 0 || value > 100000) {
        return 0;
    }

    *out = (int)value;
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
    if (ip_header_len < 20 || len < 14 + ip_header_len + 8 || total_len < ip_header_len + 8) {
        return 0;
    }

    return frame[14 + ip_header_len] == type;
}

static void log_frame(const char* dir,
                      const uint8_t* self_mac,
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
    cfg->adapter_index = -1;
    cfg->verbose = 1;
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

static int parse_args(int argc, char* argv[], app_config_t* cfg, int* list_only)
{
    init_config(cfg);
    *list_only = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "--list") == 0) {
            *list_only = 1;
        } else if (strcmp(argv[i], "--adapter") == 0 && i + 1 < argc) {
            cfg->adapter = argv[++i];
            parse_int(cfg->adapter, &cfg->adapter_index);
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
        } else {
            fprintf(stderr, "unknown or incomplete option: %s\n", argv[i]);
            return 0;
        }
    }

    return 1;
}

static void list_adapters(pcap_api_t* pcap)
{
    pcap_if_t* alldevs = NULL;
    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    int index = 1;

    if (pcap->findalldevs(&alldevs, errbuf) != 0) {
        fprintf(stderr, "pcap_findalldevs failed: %s\n", errbuf);
        return;
    }

    printf("%s\n", pcap->lib_version());
    for (pcap_if_t* dev = alldevs; dev; dev = dev->next) {
        printf("%2d. %s\n", index++, dev->description ? dev->description : "(no description)");
        printf("    %s\n", dev->name ? dev->name : "(no name)");
    }

    pcap->freealldevs(alldevs);
}

static const char* select_adapter_name(pcap_api_t* pcap, const app_config_t* cfg)
{
    pcap_if_t* alldevs = NULL;
    pcap_if_t* dev;
    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    int index = 1;
    const char* selected = NULL;

    if (cfg->adapter && cfg->adapter_index < 0) {
        return cfg->adapter;
    }

    if (pcap->findalldevs(&alldevs, errbuf) != 0) {
        fprintf(stderr, "pcap_findalldevs failed: %s\n", errbuf);
        return NULL;
    }

    if (cfg->adapter_index < 0) {
        fprintf(stderr, "missing --adapter. Use --list first.\n");
        pcap->freealldevs(alldevs);
        return NULL;
    }

    for (dev = alldevs; dev; dev = dev->next, ++index) {
        if (index == cfg->adapter_index) {
            selected = _strdup(dev->name);
            break;
        }
    }

    pcap->freealldevs(alldevs);

    if (!selected) {
        fprintf(stderr, "adapter index not found: %d\n", cfg->adapter_index);
    }

    return selected;
}

static err_t pcap_link_output(void* ctx, const uint8_t* frame, uint16_t frame_len)
{
    app_context_t* app = (app_context_t*)ctx;

    ++app->tx_count;
    if (app->verbose) {
        log_frame("TX", app->mac, app->ip, frame, frame_len);
    }

    if (app->pcap->sendpacket(app->handle, frame, frame_len) != 0) {
        fprintf(stderr, "pcap_sendpacket failed: %s\n", app->pcap->geterr(app->handle));
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

int main(int argc, char* argv[])
{
    pcap_api_t pcap;
    pcap_t* handle;
    const char* adapter_name;
    app_config_t cfg;
    app_context_t app;
    mipos_lwip_netif_t iface;
    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    int list_only;

    if (!parse_args(argc, argv, &cfg, &list_only)) {
        usage(argv[0]);
        return 2;
    }

    if (!load_pcap(&pcap)) {
        return 1;
    }

    if (list_only) {
        list_adapters(&pcap);
        unload_pcap(&pcap);
        return 0;
    }

    adapter_name = select_adapter_name(&pcap, &cfg);
    if (!adapter_name) {
        unload_pcap(&pcap);
        return 2;
    }

    handle = pcap.open_live(adapter_name, 65536, 1, 1, errbuf);
    if (!handle) {
        fprintf(stderr, "pcap_open_live failed: %s\n", errbuf);
        if (adapter_name != cfg.adapter) {
            free((void*)adapter_name);
        }
        unload_pcap(&pcap);
        return 1;
    }

    lwip_init();

    memset(&app, 0, sizeof(app));
    app.pcap = &pcap;
    app.handle = handle;
    app.verbose = cfg.verbose;
    memcpy(app.mac, cfg.mac, sizeof(app.mac));
    app.ip[0] = ip4_addr1(&cfg.ip);
    app.ip[1] = ip4_addr2(&cfg.ip);
    app.ip[2] = ip4_addr3(&cfg.ip);
    app.ip[3] = ip4_addr4(&cfg.ip);

    if (mipos_lwip_netif_open(&iface,
                              cfg.mac,
                              &cfg.ip,
                              &cfg.netmask,
                              &cfg.gateway,
                              pcap_link_output,
                              &app) != ERR_OK) {
        fprintf(stderr, "mipos_lwip_netif_open failed\n");
        pcap.close(handle);
        if (adapter_name != cfg.adapter) {
            free((void*)adapter_name);
        }
        unload_pcap(&pcap);
        return 1;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    printf("adapter: %s\n", adapter_name);
    printf("mipOS MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           cfg.mac[0], cfg.mac[1], cfg.mac[2],
           cfg.mac[3], cfg.mac[4], cfg.mac[5]);
    print_ip("mipOS IP: ", &cfg.ip);
    printf("\n");
    printf("Ready. Ping the mipOS IP from Windows. Press Ctrl-C to stop.\n");
    if (cfg.verbose) {
        printf("Packet log enabled. Use --quiet to suppress RX/TX lines.\n");
    }

    while (!g_stop) {
        struct pcap_pkthdr* header = NULL;
        const unsigned char* data = NULL;
        int rc = pcap.next_ex(handle, &header, &data);

        mipos_lwip_netif_poll(&iface);

        if (rc == 0) {
            continue;
        }

        if (rc < 0) {
            fprintf(stderr, "pcap_next_ex failed: %s\n", pcap.geterr(handle));
            break;
        }

        if (!header || !data || header->caplen > 65535 ||
            is_own_ethernet_frame(cfg.mac, data, header->caplen)) {
            continue;
        }

        ++app.rx_count;
        if (app.verbose) {
            log_frame("RX", cfg.mac, app.ip, data, header->caplen);
        }

        mipos_lwip_netif_input(&iface, data, (uint16_t)header->caplen);
    }

    printf("Stopped. RX=%u TX=%u\n", app.rx_count, app.tx_count);

    pcap.close(handle);
    if (adapter_name != cfg.adapter) {
        free((void*)adapter_name);
    }
    unload_pcap(&pcap);

    return 0;
}
