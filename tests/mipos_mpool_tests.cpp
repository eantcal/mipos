#include <array>
#include <cstddef>

#include <gtest/gtest.h>

extern "C" {
#include "mipos_mpool.h"
}

TEST(MiposMemoryPool, AllocatesAllBlocksAndReusesFreedBlock)
{
    constexpr std::size_t block_size = 32;
    constexpr std::size_t block_count = 3;

    alignas(void*) std::array<unsigned char, block_size * block_count> arena{};
    mipos_mpool_t pool{};

    mipos_mpool_init(&pool, block_size, block_count, arena.data());

    EXPECT_EQ(block_count, mpool_get_free_blocks(&pool));

    void* first = mipos_mpool_alloc(&pool);
    void* second = mipos_mpool_alloc(&pool);
    void* third = mipos_mpool_alloc(&pool);

    EXPECT_NE(nullptr, first);
    EXPECT_NE(nullptr, second);
    EXPECT_NE(nullptr, third);
    EXPECT_NE(first, second);
    EXPECT_NE(second, third);
    EXPECT_EQ(nullptr, mipos_mpool_alloc(&pool));
    EXPECT_EQ(0u, mpool_get_free_blocks(&pool));

    mipos_mpool_free(&pool, second);

    EXPECT_EQ(1u, mpool_get_free_blocks(&pool));
    EXPECT_EQ(second, mipos_mpool_alloc(&pool));
}

