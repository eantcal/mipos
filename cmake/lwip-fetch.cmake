include(FetchContent)

set(MIPOS_LWIP_VERSION "2.2.1" CACHE STRING "lwIP release used by mipOS")

if(POLICY CMP0169)
    cmake_policy(PUSH)
    cmake_policy(SET CMP0169 OLD)
endif()

FetchContent_Declare(
    lwip
    URL "https://download-mirror.savannah.gnu.org/releases/lwip/lwip-${MIPOS_LWIP_VERSION}.zip"
    URL_HASH SHA256=7b622662dba2383d71f874f2e494b54ae948531559c17acbe75797966d646878
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(lwip)
if(NOT lwip_POPULATED)
    FetchContent_Populate(lwip)
endif()

if(POLICY CMP0169)
    cmake_policy(POP)
endif()

set(MIPOS_LWIP_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/mipos/net/lwip
    ${lwip_SOURCE_DIR}/src/include
)

set(MIPOS_LWIP_SOURCES
    ${lwip_SOURCE_DIR}/src/core/def.c
    ${lwip_SOURCE_DIR}/src/core/inet_chksum.c
    ${lwip_SOURCE_DIR}/src/core/init.c
    ${lwip_SOURCE_DIR}/src/core/ip.c
    ${lwip_SOURCE_DIR}/src/core/mem.c
    ${lwip_SOURCE_DIR}/src/core/memp.c
    ${lwip_SOURCE_DIR}/src/core/netif.c
    ${lwip_SOURCE_DIR}/src/core/pbuf.c
    ${lwip_SOURCE_DIR}/src/core/raw.c
    ${lwip_SOURCE_DIR}/src/core/stats.c
    ${lwip_SOURCE_DIR}/src/core/sys.c
    ${lwip_SOURCE_DIR}/src/core/tcp.c
    ${lwip_SOURCE_DIR}/src/core/tcp_in.c
    ${lwip_SOURCE_DIR}/src/core/tcp_out.c
    ${lwip_SOURCE_DIR}/src/core/timeouts.c
    ${lwip_SOURCE_DIR}/src/core/udp.c
    ${lwip_SOURCE_DIR}/src/core/dns.c
    ${lwip_SOURCE_DIR}/src/core/ipv4/dhcp.c
    ${lwip_SOURCE_DIR}/src/core/ipv4/etharp.c
    ${lwip_SOURCE_DIR}/src/core/ipv4/icmp.c
    ${lwip_SOURCE_DIR}/src/core/ipv4/ip4.c
    ${lwip_SOURCE_DIR}/src/core/ipv4/ip4_addr.c
    ${lwip_SOURCE_DIR}/src/core/ipv4/ip4_frag.c
    ${lwip_SOURCE_DIR}/src/netif/ethernet.c
)
