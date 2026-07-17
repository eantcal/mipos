#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

extern "C" {
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/init.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "mipos_lwip_netif.h"
}

namespace {

constexpr std::array<uint8_t, 6> kMipMac{{0x02, 0x00, 0x00, 0x00, 0x00, 0x01}};
constexpr std::array<uint8_t, 6> kHostMac{{0x02, 0x00, 0x00, 0x00, 0x00, 0x02}};
constexpr std::array<uint8_t, 4> kMipIp{{192, 168, 1, 2}};
constexpr std::array<uint8_t, 4> kHostIp{{192, 168, 1, 10}};

struct CapturedFrames {
    std::vector<std::vector<uint8_t>> frames;
};

uint16_t read_be16(const uint8_t* data)
{
    return static_cast<uint16_t>((data[0] << 8) | data[1]);
}

void write_be16(std::vector<uint8_t>& frame, size_t off, uint16_t value)
{
    frame[off] = static_cast<uint8_t>(value >> 8);
    frame[off + 1] = static_cast<uint8_t>(value & 0xffu);
}

void write_be32(std::vector<uint8_t>& frame, size_t off, const std::array<uint8_t, 4>& value)
{
    std::memcpy(frame.data() + off, value.data(), value.size());
}

uint16_t checksum(const uint8_t* data, size_t len)
{
    uint32_t sum = 0;

    while (len > 1) {
        sum += read_be16(data);
        data += 2;
        len -= 2;
    }

    if (len) {
        sum += static_cast<uint16_t>(data[0] << 8);
    }

    while (sum >> 16) {
        sum = (sum & 0xffffu) + (sum >> 16);
    }

    return static_cast<uint16_t>(~sum);
}

err_t capture_link_output(void* ctx, const uint8_t* frame, uint16_t frame_len)
{
    auto* captured = static_cast<CapturedFrames*>(ctx);
    captured->frames.emplace_back(frame, frame + frame_len);
    return ERR_OK;
}

std::vector<uint8_t> make_arp_request()
{
    std::vector<uint8_t> frame(14 + 28, 0);

    std::memset(frame.data(), 0xff, 6);
    std::memcpy(frame.data() + 6, kHostMac.data(), 6);
    write_be16(frame, 12, 0x0806);

    write_be16(frame, 14, 1);
    write_be16(frame, 16, 0x0800);
    frame[18] = 6;
    frame[19] = 4;
    write_be16(frame, 20, 1);
    std::memcpy(frame.data() + 22, kHostMac.data(), 6);
    write_be32(frame, 28, kHostIp);
    write_be32(frame, 38, kMipIp);

    return frame;
}

std::vector<uint8_t> make_icmp_echo_request()
{
    std::vector<uint8_t> frame(14 + 20 + 8, 0);

    std::memcpy(frame.data(), kMipMac.data(), 6);
    std::memcpy(frame.data() + 6, kHostMac.data(), 6);
    write_be16(frame, 12, 0x0800);

    const size_t ip = 14;
    frame[ip] = 0x45;
    write_be16(frame, ip + 2, 28);
    write_be16(frame, ip + 4, 0x1234);
    frame[ip + 8] = 64;
    frame[ip + 9] = 1;
    write_be32(frame, ip + 12, kHostIp);
    write_be32(frame, ip + 16, kMipIp);
    write_be16(frame, ip + 10, checksum(frame.data() + ip, 20));

    const size_t icmp = ip + 20;
    frame[icmp] = 8;
    write_be16(frame, icmp + 4, 0x4567);
    write_be16(frame, icmp + 6, 1);
    write_be16(frame, icmp + 2, checksum(frame.data() + icmp, 8));

    return frame;
}

bool is_ipv4_proto(const std::vector<uint8_t>& frame, uint8_t proto)
{
    return frame.size() >= 34 && read_be16(frame.data() + 12) == 0x0800 &&
           frame[23] == proto;
}

uint16_t ipv4_payload_offset(const std::vector<uint8_t>& frame)
{
    return static_cast<uint16_t>(14u + ((frame[14] & 0x0fu) * 4u));
}

bool is_udp_ports(const std::vector<uint8_t>& frame, uint16_t src, uint16_t dst)
{
    if (!is_ipv4_proto(frame, 17)) {
        return false;
    }

    const uint16_t udp = ipv4_payload_offset(frame);
    return frame.size() >= udp + 8 && read_be16(frame.data() + udp) == src &&
           read_be16(frame.data() + udp + 2) == dst;
}

bool has_frame_matching(const CapturedFrames& captured,
                        bool (*predicate)(const std::vector<uint8_t>&))
{
    for (const auto& frame : captured.frames) {
        if (predicate(frame)) {
            return true;
        }
    }

    return false;
}

bool is_dns_query_to_host(const std::vector<uint8_t>& frame)
{
    return frame.size() >= 42 && is_udp_ports(frame, read_be16(frame.data() + ipv4_payload_offset(frame)), 53) &&
           std::memcmp(frame.data() + 30, kHostIp.data(), 4) == 0;
}

bool is_dhcp_discover(const std::vector<uint8_t>& frame)
{
    return is_udp_ports(frame, 68, 67);
}

err_t tcp_connected_noop(void* arg, struct tcp_pcb* pcb, err_t err)
{
    (void)arg;
    (void)pcb;
    return err;
}

void dns_found_noop(const char* name, const ip_addr_t* ipaddr, void* arg)
{
    (void)name;
    (void)ipaddr;
    (void)arg;
}

}  // namespace

TEST(MiposLwip, SupportsIpv4ArpIcmpUdpTcpDhcpAndDns)
{
    lwip_init();

    CapturedFrames captured;
    mipos_lwip_netif_t iface{};
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gateway;

    IP4_ADDR(&ipaddr, 192, 168, 1, 2);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gateway, 192, 168, 1, 1);

    ASSERT_EQ(ERR_OK,
              mipos_lwip_netif_open(&iface,
                                    kMipMac.data(),
                                    &ipaddr,
                                    &netmask,
                                    &gateway,
                                    capture_link_output,
                                    &captured));

    captured.frames.clear();
    ASSERT_EQ(ERR_OK, mipos_lwip_netif_input(&iface, make_arp_request().data(), 42));
    ASSERT_EQ(1u, captured.frames.size());

    const auto& arp = captured.frames[0];
    ASSERT_GE(arp.size(), 42u);
    EXPECT_EQ(0, std::memcmp(arp.data(), kHostMac.data(), 6));
    EXPECT_EQ(0, std::memcmp(arp.data() + 6, kMipMac.data(), 6));
    EXPECT_EQ(0x0806u, read_be16(arp.data() + 12));
    EXPECT_EQ(2u, read_be16(arp.data() + 20));
    EXPECT_EQ(0, std::memcmp(arp.data() + 22, kMipMac.data(), 6));
    EXPECT_EQ(0, std::memcmp(arp.data() + 28, kMipIp.data(), 4));

    captured.frames.clear();
    auto echo = make_icmp_echo_request();
    ASSERT_EQ(ERR_OK, mipos_lwip_netif_input(&iface, echo.data(), static_cast<uint16_t>(echo.size())));
    ASSERT_EQ(1u, captured.frames.size());

    const auto& reply = captured.frames[0];
    ASSERT_GE(reply.size(), 42u);
    EXPECT_EQ(0, std::memcmp(reply.data(), kHostMac.data(), 6));
    EXPECT_EQ(0, std::memcmp(reply.data() + 6, kMipMac.data(), 6));
    EXPECT_EQ(0x0800u, read_be16(reply.data() + 12));
    EXPECT_EQ(1u, reply[23]);
    EXPECT_EQ(0, std::memcmp(reply.data() + 26, kMipIp.data(), 4));
    EXPECT_EQ(0, std::memcmp(reply.data() + 30, kHostIp.data(), 4));
    EXPECT_EQ(0u, reply[34]);
    EXPECT_EQ(0u, reply[35]);
    EXPECT_EQ(0x4567u, read_be16(reply.data() + 38));
    EXPECT_EQ(1u, read_be16(reply.data() + 40));

    captured.frames.clear();
    struct udp_pcb* udp = udp_new();
    ASSERT_NE(nullptr, udp);
    ip4_addr_t host_ip4;
    ip_addr_t host_addr;
    IP4_ADDR(&host_ip4, kHostIp[0], kHostIp[1], kHostIp[2], kHostIp[3]);
    ip_addr_copy_from_ip4(host_addr, host_ip4);
    ASSERT_EQ(ERR_OK, udp_bind(udp, IP_ADDR_ANY, 12000));
    struct pbuf* udp_payload = pbuf_alloc(PBUF_TRANSPORT, 4, PBUF_RAM);
    ASSERT_NE(nullptr, udp_payload);
    ASSERT_EQ(ERR_OK, pbuf_take(udp_payload, "mipo", 4));
    ASSERT_EQ(ERR_OK, udp_sendto(udp, udp_payload, &host_addr, 12001));
    pbuf_free(udp_payload);
    udp_remove(udp);
    ASSERT_FALSE(captured.frames.empty());
    EXPECT_TRUE(has_frame_matching(captured, [](const std::vector<uint8_t>& frame) {
        return is_udp_ports(frame, 12000, 12001);
    }));

    captured.frames.clear();
    struct tcp_pcb* tcp = tcp_new();
    ASSERT_NE(nullptr, tcp);
    ASSERT_EQ(ERR_OK, tcp_bind(tcp, IP_ADDR_ANY, 10000));
    ASSERT_EQ(ERR_OK, tcp_connect(tcp, &host_addr, 80, tcp_connected_noop));
    ASSERT_FALSE(captured.frames.empty());
    EXPECT_TRUE(has_frame_matching(captured, [](const std::vector<uint8_t>& frame) {
        return is_ipv4_proto(frame, 6);
    }));
    tcp_abort(tcp);

    struct tcp_pcb* listener = tcp_new();
    ASSERT_NE(nullptr, listener);
    ASSERT_EQ(ERR_OK, tcp_bind(listener, IP_ADDR_ANY, 8080));
    struct tcp_pcb* listen_pcb = tcp_listen(listener);
    ASSERT_NE(nullptr, listen_pcb);
    tcp_close(listen_pcb);

    captured.frames.clear();
    dns_setserver(0, &host_addr);
    ip_addr_t resolved;
    ASSERT_EQ(ERR_INPROGRESS,
              dns_gethostbyname("example.com", &resolved, dns_found_noop, nullptr));
    ASSERT_FALSE(captured.frames.empty());
    EXPECT_TRUE(has_frame_matching(captured, is_dns_query_to_host));

    captured.frames.clear();
    ASSERT_EQ(ERR_OK, dhcp_start(&iface.netif));
    ASSERT_FALSE(captured.frames.empty());
    EXPECT_TRUE(has_frame_matching(captured, is_dhcp_discover));
    dhcp_stop(&iface.netif);
}
