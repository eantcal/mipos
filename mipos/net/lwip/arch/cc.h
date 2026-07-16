#ifndef MIPOS_LWIP_ARCH_CC_H
#define MIPOS_LWIP_ARCH_CC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#pragma warning(disable: 4103)
#endif

#ifndef BYTE_ORDER
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif

typedef int sys_prot_t;

#define LWIP_PLATFORM_DIAG(x) do { printf x; } while (0)
#define LWIP_PLATFORM_ASSERT(x) do { \
    printf("lwIP assert: %s\n", (x)); \
    abort(); \
} while (0)

#define PACK_STRUCT_USE_INCLUDES

#define LWIP_RAND() ((uint32_t)0x12345678u)

#endif
