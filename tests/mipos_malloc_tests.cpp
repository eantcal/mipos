#include <array>
#include <cstddef>
#include <cstring>

#include <gtest/gtest.h>

extern "C" {
#include "mipos_malloc.h"
}

TEST(MiposMalloc, AllocFreeAndReallocPreservesContents)
{
    alignas(std::max_align_t) std::array<unsigned char, 512> arena{};
    mipos_mm_t mm{};

    mipos_mm_init(&mm, arena.data(), arena.size());

    void* block = mipos_mm_alloc(&mm, 16);
    ASSERT_NE(nullptr, block);
    EXPECT_TRUE(mipos_mm_is_in(&mm, block));
    EXPECT_GT(mipos_mm_get_used_memory_size(&mm), 0u);

    std::memset(block, 0x5A, 16);

    void* grown = mipos_mm_realloc(&mm, block, 64);
    ASSERT_NE(nullptr, grown);
    EXPECT_TRUE(mipos_mm_is_in(&mm, grown));
    EXPECT_EQ(0, std::memcmp(grown, std::array<unsigned char, 16>{
                                      0x5A, 0x5A, 0x5A, 0x5A,
                                      0x5A, 0x5A, 0x5A, 0x5A,
                                      0x5A, 0x5A, 0x5A, 0x5A,
                                      0x5A, 0x5A, 0x5A, 0x5A}
                                      .data(),
                              16));

    EXPECT_EQ(1, mipos_mm_free(&mm, grown));
    EXPECT_FALSE(mipos_mm_is_in(&mm, grown));
}

