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


eco_namespace(eco);
eco_namespace(lockfree);
////////////////////////////////////////////////////////////////////////////////
struct flow
{
private:
    uint32_t    m_size;
    uint32_t    m_curr;
    std::atomic<uint32_t> m_ref;

    struct node
    {
        eco::offset_t   flow;
        char            data[];
    };

public:
    inline void* move(
        uint32_t size,
        eco::lockfree::freepool& pool)
    {
        if (m_curr + size < m_size)
        {
            m_curr += size;
            ++m_ref;
            char* c = reinterpret_cast<char*>(this) + m_curr;
            node* n = reinterpret_cast<node*>(c);
            n->flow = pool.offset(this);
            return n->data;
        }
        return NULL;
    }

    static inline eco::lockfree::flow* get(
        void* obj, 
        eco::lockfree::freepool& pool)
    {
        char* c = static_cast<char*>(obj) - offsetof(node, data);
        node* n = reinterpret_cast<node*>(c);
        return pool.ptr<flow>(n->flow);
    }

    inline bool unref()
    {
        return --m_ref == 0;
    }
};


////////////////////////////////////////////////////////////////////////////////
template<typename stack_t = eco::lockfree::stack_mc>
class flowlist
{
public:
    typedef typename eco::lockfree::freelist<stack_t> freelist_t;

    inline flowlist(eco::lockfree::cachepool& pool, uint32_t size = 0);

    inline flowlist(eco::lockfree::spanpool& pool, uint32_t size = 0)
        : m_pool(pool)
    {
        count(size);
    }

    inline eco::lockfree::freepool& freepool()
    {
        return m_pool.freepool();
    }

    inline uint32_t count(uint32_t size)
    {
        if (m_flowsize)
        {
            m_flowsize = m_flowsize - (m_flowsize >> FLOW_BATCH_BIT);
            m_flowsize += size;
        }
        else
        {
            m_flowsize = (size << FLOW_BATCH_BIT);
        }
        return m_flowsize + sizeof(eco::lockfree::flow);
    }

    inline bool ready(typename freelist_t::ptr& fl, uint32_t size)
    {
        return fl && (fl->size() < size);
    }

    inline void* alloc(uint32_t size)
    {
        // average size: is average size of recently allocate 64 object.
        size = align_up(size);
        uint32_t flowsize = count(size);

        // reset freelist obj size when 1st use or freelist size is too small
        // freelist.size < avgsize * (batch / 2) // TODO
        uint32_t flowsize_min = flowsize / 2;
        typename freelist_t::ptr fl(freelist_get());
        while (!ready(fl, flowsize_min))
        {
            eco::spinlock lock(m_mutex);
            fl = freelist_get();
            if (!ready(fl, size))
            {
                if (!resize(size)) { return NULL; }
                fl = freelist_get();
            }
        }

        // alloc object from flow object.
        auto* flw = static_cast<eco::lockfree::flow*>(fl->alloc());
        for (void* obj = NULL; flw != NULL; )
        {
            obj = flw->move(size, m_pool.freepool());
            if (obj)
            {
                fl->dealloc(flw);
                return obj;
            }
            flw = static_cast<eco::lockfree::flow*>(fl->alloc());
        }
        return NULL;
    }

    inline void dealloc(void* obj)
    {
        auto* flw = eco::lockfree::flow::get(obj, m_pool.freepool());
        if (flw->unref())
        {
            typename freelist_t::ptr fl(freelist_get());
            fl->dealloc_or_release(flw);
        }
    }

private:
    inline eco::bool_t resize(uint32_t size)
    {
        size = size + (size >> 1);
        typename freelist_t::ptr fl = make_freelist<stack_t>(m_pool, size, 32);
        if (fl)
        {
            uint32_t fl_new = m_pool.freepool().offset(fl.release());
            uint32_t fl_old = m_freelist.exchange(fl_new);
            // old freelist unref
            m_pool.freepool().ptr<freelist_t>(fl_old)->unref();
        }
        return (fl != 0);
    }

    inline typename freelist_t::ptr& freelist_get()
    {
        return m_pool.freepool().ptr<freelist_t>(m_freelist.load());
    }

private:
    static const uint32_t FLOW_BATCH = 64;
    static const uint32_t FLOW_BATCH_BIT = 6;
    static const uint32_t FLOW_NUM = 8;
    uint32_t        m_flowsize = 0;
    std::atomic<eco::offset_t> m_freelist;
    eco::spinmutex  m_mutex;
    eco::lockfree::spanpool& m_pool;
};



////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(lockfree);
eco_namespace_end(eco);