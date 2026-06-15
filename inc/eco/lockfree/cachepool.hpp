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
#include <eco/lockfree/freelist.hpp>
#include <eco/lockfree/flexlist.hpp>
#include <eco/lockfree/flowlist.hpp>


eco_namespace(eco);
eco_namespace(lockfree);
////////////////////////////////////////////////////////////////////////////////
class cachepool
{
public:
    inline cachepool()
        : m_metapool(m_freepool, 
                     sizeof(eco::lockfree::span),
                     sizeof(eco::lockfree::freelist<>),
                     page_radix_l1::size())
        , m_spanpool(m_freepool, m_metapool)
    {}

    inline cachepool(eco::lockfree::freepolicy& policy)
        : m_metapool(m_freepool, 
                     sizeof(eco::lockfree::span),
                     sizeof(eco::lockfree::freelist<>),
                     page_radix_l1::size())
        , m_spanpool(m_freepool, m_metapool)
    {
        init(policy);
    }

    inline ~cachepool()
    {}

    inline bool init(eco::lockfree::freepolicy& policy)
    {
        return m_spanpool.init(policy);
    }

    inline bool ready() const
    {
        return m_freepool.ready();
    }

    template<typename object_t>
    inline object_t* ptr(uint32_t offset)
    {
        return m_freepool.ptr<object_t>(offset);
    }

    inline void* ptr(uint32_t offset)
    {
        return m_freepool.ptr(offset);
    }

    inline eco::offset_t offset(void* ptr)
    {
        return m_freepool.offset(ptr);
    }

    inline char* base()
    {
        return m_freepool.base();
    }

    inline eco::lockfree::spanpool& spanpool()
    {
        return m_spanpool;
    }

    template<typename stack_t = eco::lockfree::stack_mc>
    inline typename eco::lockfree::freelist<stack_t>::ptr make_freelist(
        uint32_t size, uint32_t batch)
    {
        return eco::lockfree::make_freelist<stack_t>(m_spanpool, size, batch);
    }

private:
    eco::lockfree::freepool m_freepool;
    eco::lockfree::metapool m_metapool;
    eco::lockfree::spanpool m_spanpool;
};


////////////////////////////////////////////////////////////////////////////////
template<typename stack_t>
freelist<stack_t>::freelist(cachepool& p, uint32_t size, uint32_t batch)
    : stack_t(p.base())
    , m_pool(p.spanpool())
    , m_size(eco::align_up(size, 4))
    , m_batch(m_pool.recount_batch(m_size, batch))
{
    alloc_batch();
}
////////////////////////////////////////////////////////////////////////////////
template<typename stack_t>
flexlist<stack_t>::flexlist(cachepool& p, uint32_t size) : m_pool(p.spanpool())
{
    size  = eco::align_up(size, 4);
    if (size > 0) { resize(size); }
}
////////////////////////////////////////////////////////////////////////////////
template<typename stack_t>
flowlist<stack_t>::flowlist(cachepool& p, uint32_t size) : m_pool(p.spanpool())
{
    count(size);
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(lockfree);
eco_namespace_end(eco);