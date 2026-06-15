#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/lockfree/cachepool.hpp>
#include <./inc/number.hpp>


////////////////////////////////////////////////////////////////////////////////
class freepolicy : public ::testing::Test {};
class freepool : public ::testing::Test
{
public:
    inline void link_test(
        eco::lockfree::freepool& pool,
        uint32_t size,
        uint32_t total,
        uint32_t first_pos)
    {
        uint32_t count = 0;
        uint32_t link = (total / size - 1);
        char* first = pool.ptr(first_pos);
        void* last = static_cast<char*>(
            eco::lockfree::stack_mc::link(first, first_pos, total, size));
        ASSERT_EQ(last, first + size * link);
        
        // validate obj.next
        for (char* i = first; i < last; i += size)
        {
            uint32_t* obj = reinterpret_cast<uint32_t*>(i);
            ASSERT_EQ(*obj, pool.offset(i) + size);
            count += 1;
        }
        ASSERT_EQ(*static_cast<uint32_t*>(last), 0u);
        ASSERT_EQ(count, link);
    }
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(freepolicy, memory_size)
{
    eco::lockfree::freepolicy policy;
    // default policy value
    ASSERT_EQ(policy.memory_size(), 8u * 1024 * 1024);
    
    // memory_size == 1 page, when it < page_size
    uint32_t psize = policy.page_size();
    policy.memory_size(0);
    ASSERT_EQ(policy.memory_size(), psize);
    policy.memory_size(1);
    ASSERT_EQ(policy.memory_size(), psize);
    policy.memory_size(psize - 1);
    ASSERT_EQ(policy.memory_size(), psize);
    policy.memory_size(psize);
    ASSERT_EQ(policy.memory_size(), psize);

    // memory_size = n page (n is uint32)
    policy.memory_size(k1_MIN(psize));
    ASSERT_EQ(policy.memory_size(), psize * 2);
    policy.memory_size(k1_5(psize));
    ASSERT_EQ(policy.memory_size(), psize * 2);
    policy.memory_size(k1_75(psize));
    ASSERT_EQ(policy.memory_size(), psize * 2);
    policy.memory_size(k1_875(psize));
    ASSERT_EQ(policy.memory_size(), psize * 2);
    policy.memory_size(k1_875(psize));
    ASSERT_EQ(policy.memory_size(), psize * 2);
    policy.memory_size(k1_MAX(psize));
    ASSERT_EQ(policy.memory_size(), psize * 2);
    policy.memory_size(psize * 3);
    ASSERT_EQ(policy.memory_size(), psize * 3);
}
TEST_F(freepolicy, pagesize)
{
    eco::lockfree::freepolicy policy;
    const uint32_t SIZE_4K = 4 * 1024;

    // page size
    uint32_t psize = policy.page_size();
    ASSERT_EQ(psize, SIZE_4K);
    // size_to_page
    ASSERT_EQ(policy.size_to_page(0u), 0u);
    ASSERT_EQ(policy.size_to_page(1u), 1u);
    for (uint32_t i = 1; i < 10; i++)
    {
        uint32_t size = psize * i;
        ASSERT_EQ(policy.size_to_page(size), i);
        ASSERT_EQ(policy.size_to_page(size + k1_MIN(psize)), i + 2);
        ASSERT_EQ(policy.size_to_page(size + k1_5(psize)), i + 2);
        ASSERT_EQ(policy.size_to_page(size + k1_75(psize)), i + 2);
        ASSERT_EQ(policy.size_to_page(size + k1_875(psize)), i + 2);
    }

    // round_page_size
    ASSERT_EQ(policy.round_page_size(0u), 0u);
    ASSERT_EQ(policy.round_page_size(1u), psize);
    ASSERT_EQ(policy.round_page_size(psize), psize);
    for (uint32_t i = 1; i < 10; i++)
    {
        uint32_t size = psize * i;
        ASSERT_EQ(policy.round_page_size(size), size);
        ASSERT_EQ(policy.round_page_size(size + k1_MIN(psize)), psize * (i + 2));
        ASSERT_EQ(policy.round_page_size(size + k1_5(psize)), psize * (i + 2));
        ASSERT_EQ(policy.round_page_size(size + k1_75(psize)), psize * (i + 2));
        ASSERT_EQ(policy.round_page_size(size + k1_875(psize)), psize * (i + 2));
    }

    // pageid
    ASSERT_EQ(policy.pageid(0u), 0u);
    ASSERT_EQ(policy.pageid(1u), 0u);
    ASSERT_EQ(policy.pageid(psize), 1u);
    ASSERT_EQ(policy.pageid(k1_MIN(psize)), 1u);
    ASSERT_EQ(policy.pageid(k1_5(psize)), 1u);
    ASSERT_EQ(policy.pageid(k1_75(psize)), 1u);
    ASSERT_EQ(policy.pageid(k1_875(psize)), 1u);
}
TEST_F(freepool, alloc_and_link)
{
    eco::lockfree::freepolicy policy;
    policy.memory_size(4u * 1024 * 1024);
    eco::lockfree::freepool pool;
    ASSERT_TRUE(pool.init(policy));
    char* ptr = pool.base_actual();

    // one page: ptr
    const uint32_t ONE_PAGE = 4u * 1024;
    char* obj = pool.alloc_ptr(ONE_PAGE);
    EXPECT_EQ(pool.offset(obj), ONE_PAGE);
    EXPECT_EQ(ptr, obj);
    // two page: offset
    uint32_t offset = pool.alloc(ONE_PAGE);
    EXPECT_EQ(offset, ONE_PAGE * 2);
    EXPECT_EQ(pool.ptr(offset), ptr += ONE_PAGE);
    // all page: ptr
    uint32_t page = policy.memory_size() / ONE_PAGE;
    for (uint32_t i = 2; i < page; i++)
    {
        ptr += ONE_PAGE;
        obj = pool.alloc_ptr(ONE_PAGE);
        EXPECT_EQ(obj, ptr);
    }
    // memory out
    EXPECT_EQ(pool.alloc(4), 0u);
    EXPECT_EQ(pool.alloc_ptr(4), nullptr);
    // null
    EXPECT_EQ(pool.ptr(0), nullptr);
    EXPECT_EQ(pool.offset(NULL), 0u);

    // object link: n object has (n-1) link
    link_test(pool, 4, 4096, ONE_PAGE);
    link_test(pool, 4, 4097, ONE_PAGE);
    link_test(pool, 8, 4096, ONE_PAGE);
    link_test(pool, 8, 4099, ONE_PAGE);
}
