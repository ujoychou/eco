#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/lockfree/queue.hpp>
#include <eco/test/timer.hpp>
#include <thread>


////////////////////////////////////////////////////////////////////////////////
class Fifo_mpmc : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {}

    static void TearDownTestSuite()
    {}
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(Fifo_mpmc, push_pop)
{
    const uint32_t CAPACITY = 10;
    eco::lockfree::fifo_mpmc q(CAPACITY);

    // [1] pop empty
    ASSERT_EQ(q.pop_front(), nullptr);
    ASSERT_EQ(q.pop_front(), nullptr);
    // [1] push and pop
    for (uint32_t i = 0; i < CAPACITY; i++)
    {
        ASSERT_TRUE(q.push_back(reinterpret_cast<void*>(i)));
    }
    // [1] push full
    ASSERT_FALSE(q.push_back(reinterpret_cast<void*>(0)));
    for (uint32_t i = 0; i < CAPACITY; i++)
    {
        void* obj = q.pop_front();
        uint64_t j = reinterpret_cast<uint64_t>(obj);
        ASSERT_EQ(static_cast<uint32_t>(j), i);
    }

    // [2] pop empty
    ASSERT_EQ(q.pop_front(), nullptr);
    ASSERT_EQ(q.pop_front(), nullptr);
    // [2] push and pop
    for (uint32_t i = 0; i < CAPACITY; i++)
    {
        ASSERT_TRUE(q.push_back(reinterpret_cast<void*>(i)));
    }
    // [2] push full
    ASSERT_FALSE(q.push_back(reinterpret_cast<void*>(0)));
    for (uint32_t i = 0; i < CAPACITY; i++)
    {
        void* obj = q.pop_front();
        uint64_t j = reinterpret_cast<uint64_t>(obj);
        ASSERT_EQ(static_cast<uint32_t>(j), i);
    }
}

