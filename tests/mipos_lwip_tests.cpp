#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

extern "C" {
#include "lwip/init.h"
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

}  // namespace

TEST(MiposLwip, RepliesToArpAndIcmpEcho)
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
}
