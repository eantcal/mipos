#include <gtest/gtest.h>

extern "C" {
#include "mipos_queue.h"
}

TEST(MiposQueue, PreservesFifoOrderAndReportsCapacity)
{
    mipos_queue_t queue{};
    mipos_q_item_t storage[3]{};

    mipos_q_init(&queue, storage, 3);

    EXPECT_EQ(0u, mipos_q_count(&queue));
    EXPECT_EQ(0, mipos_q_message_pending(&queue));

    EXPECT_EQ(1u, mipos_q_send(&queue, 10));
    EXPECT_EQ(2u, mipos_q_send(&queue, 20));
    EXPECT_EQ(3u, mipos_q_send(&queue, 30));
    EXPECT_EQ(static_cast<mipos_q_size_t>(-1), mipos_q_send(&queue, 40));
    EXPECT_EQ(3u, mipos_q_count(&queue));
    EXPECT_NE(0, mipos_q_message_pending(&queue));

    mipos_q_item_t item = 0;
    EXPECT_EQ(1u, mipos_q_receive(&queue, &item, FALSE));
    EXPECT_EQ(10u, item);
    EXPECT_EQ(1u, mipos_q_receive(&queue, &item, FALSE));
    EXPECT_EQ(20u, item);
    EXPECT_EQ(1u, mipos_q_receive(&queue, &item, FALSE));
    EXPECT_EQ(30u, item);

    EXPECT_EQ(0u, mipos_q_receive(&queue, &item, FALSE));
    EXPECT_EQ(0u, mipos_q_count(&queue));
}

