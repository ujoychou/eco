#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/lockfree/cachepool.hpp>


////////////////////////////////////////////////////////////////////////////////
class spanpool : public ::testing::Test
{
public:
    inline void test_span_alloc(
        eco::lockfree::spanpool& pool, 
        uint32_t size, 
        uint32_t batch,
        eco::bool_t recycle)
    {
        eco::lockfree::span* span = pool.span_alloc(size, batch, this);
        if (span != NULL)
        {
            ASSERT_EQ(span->refc(), batch);
            eco::lockfree::span::range range(span->data(), 0);
            range.last = eco::lockfree::stack_mc::link(span->data(),
                pool.freepool().offset(span->data()), size * batch, size);

            eco::lockfree::span* curr = NULL;
            char* i = static_cast<char*>(range.first);
            for (; i <= static_cast<char*>(range.last); i += size)
            {
                curr = pool.span_find(i);
                ASSERT_EQ(span, curr);
                ASSERT_EQ(span->unref(), i == range.last);
            }
            curr = pool.span_alloc(size, batch, NULL);
            ASSERT_EQ(span, curr);
            if (recycle) { span->spanlist()->dealloc(span); }
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(spanpool, spanlist)
{
    eco::lockfree::freepolicy policy;
    eco::lockfree::cachepool pool(policy);
    ASSERT_TRUE(pool.ready());
    ASSERT_TRUE(!pool.init(policy));

    // spanlist pages index
    eco::lockfree::spanpool& spanpool = pool.spanpool();
    for (uint32_t page = 1; page <= 512; page++)
    {
        eco::lockfree::spanlist* splist = spanpool.spanlist_find(page);
        uint32_t index = eco::count_highest_bit_ge(page) - 1;
        if (index > 7) { index = 7; }
        EXPECT_EQ(splist->index(), index);
        EXPECT_GE(splist->pages(), page);
    }
    EXPECT_EQ(spanpool.spanlist_hugepage()->index(), 7u);
    EXPECT_EQ(spanpool.spanlist_hugepage()->pages(), 512u);

    // spanlist alloc
    eco::lockfree::spanlist* splist = spanpool.spanlist_find(1);
    eco::lockfree::span* span1 = splist->alloc();
    EXPECT_EQ(span1, nullptr);
    span1 = splist->alloc_from_freepool(spanpool.metapool());
    ASSERT_NE(span1, nullptr);
    EXPECT_EQ(span1->pages(), splist->pages());
    splist->dealloc(span1);
    eco::lockfree::span* span2 = splist->alloc();
    EXPECT_EQ(span1, span2);
    splist->dealloc(span1);
    span2 = splist->alloc_from_freepool(spanpool.metapool());
    splist->dealloc(span2);
    auto span3 = splist->alloc_from_freepool(spanpool.metapool());
    splist->dealloc(span3);
    EXPECT_EQ(span3, splist->alloc());
    EXPECT_EQ(span2, splist->alloc());
    EXPECT_EQ(span1, splist->alloc());
    EXPECT_EQ(nullptr, splist->alloc());

    // recount_batch: 1page
    uint32_t batch = spanpool.recount_batch(4, 32);
    EXPECT_EQ(batch, 1024u);
    batch = spanpool.recount_batch(8, 64);
    EXPECT_EQ(batch, 512u);
    batch = spanpool.recount_batch(32, 32);
    EXPECT_EQ(batch, 128u);
    batch = spanpool.recount_batch(32, 128);
    EXPECT_EQ(batch, 128u);
    batch = spanpool.recount_batch(64, 64);
    EXPECT_EQ(batch, 64u);
    // recount_batch: 2page
    batch = spanpool.recount_batch(128, 63);
    EXPECT_EQ(batch, 64u);
    batch = spanpool.recount_batch(128, 64);
    EXPECT_EQ(batch, 64u);
    // recount_batch: 4page
    batch = spanpool.recount_batch(128, 65);
    EXPECT_EQ(batch, 128u);
    batch = spanpool.recount_batch(256, 64);
    EXPECT_EQ(batch, 64u);
}
TEST_F(spanpool, span_alloc_dealloc)
{
    eco::lockfree::freepolicy policy;
    eco::lockfree::cachepool pool(policy);
    ASSERT_TRUE(pool.ready());
    ASSERT_TRUE(!pool.init(policy));

    // span_alloc_and_link
    eco::lockfree::spanpool& spanpool = pool.spanpool();
    test_span_alloc(spanpool, 4, 32, false);
    test_span_alloc(spanpool, 8, 32, false);
    test_span_alloc(spanpool, 32, 127, false);
    test_span_alloc(spanpool, 32, 128, false);
    test_span_alloc(spanpool, 32, 129, false);
    test_span_alloc(spanpool, 64, 128, false);
    test_span_alloc(spanpool, 64, 127, false);
    test_span_alloc(spanpool, 64, 129, false);
    // span_alloc_and_link: recycle
    test_span_alloc(spanpool, 4, 32, true);
    test_span_alloc(spanpool, 8, 32, true);
    test_span_alloc(spanpool, 32, 127, true);
    test_span_alloc(spanpool, 32, 128, true);
    test_span_alloc(spanpool, 32, 129, true);
    test_span_alloc(spanpool, 64, 128, true);
    test_span_alloc(spanpool, 64, 127, true);
    test_span_alloc(spanpool, 64, 129, true);
}
