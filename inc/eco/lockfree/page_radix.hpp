#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-04-25] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2025 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/lockfree/freepool.hpp>


eco_namespace(eco);
eco_namespace(lockfree);
class span;
////////////////////////////////////////////////////////////////////////////////
struct page_radix_it
{
    eco::lockfree::span* span;
};

struct page_radix_l1
{
    // l1 = it * (1 << 10)
    std::atomic<eco::lockfree::page_radix_it*> items = {0};

    static inline uint32_t size()
    {
        return static_cast<uint32_t>(sizeof(page_radix_it) << 10);
    }
};

struct page_radix_l2
{
    // l2 = l1 * (1 << 10)
    eco::lockfree::page_radix_l1* radix;
};


////////////////////////////////////////////////////////////////////////////////
class page_radix_32
{
private:
    eco::spinmutex m_mutex;    
    eco::lockfree::page_radix_l1 m_level1[1024];

public:
    inline eco::lockfree::span* find(uint32_t page_id) const
    {
        // 20bits = 10 bits(l1) + 10 bits(it)
        uint32_t l1 = (page_id >> 10);
        uint32_t it = (page_id & 1023);
        eco::lockfree::page_radix_it* item = m_level1[l1].items;
        return item[it].span;
    }

    inline bool set(
        uint32_t page_id,
        eco::lockfree::span* span,
        eco::lockfree::metapool& meta)
    {
        // 20bits = 10 bits(l1) + 10 bits(it)
        uint32_t l1 = (page_id >> 10);
        uint32_t it = (page_id & 1023);

        eco::lockfree::page_radix_l1& level1 = m_level1[l1];
        eco::lockfree::page_radix_it* item = level1.items;
        if (item == NULL)
        {
            eco::spinlock lock(m_mutex);
            if (level1.items == NULL)
            {
                level1.items = static_cast<eco::lockfree::page_radix_it*>(
                    meta.radix_it_alloc());
                item = level1.items;
                if (item == NULL) { return false; }
            }
        }
        item[it].span = span;
        return true;
    }
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(lockfree);
eco_namespace_end(eco);