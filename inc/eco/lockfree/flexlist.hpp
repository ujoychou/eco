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
#include <string.h>


eco_namespace(eco);
eco_namespace(lockfree);
////////////////////////////////////////////////////////////////////////////////
template<typename stack_t = eco::lockfree::stack_mc>
class flexlist
{
public:
    typedef typename eco::lockfree::freelist<stack_t> freelist_t;

    inline flexlist(eco::lockfree::cachepool& pool, uint32_t size = 0);

    inline flexlist(eco::lockfree::spanpool& pool, uint32_t size = 0)
        : m_pool(pool)
    {
        size  = eco::align_up(size, 4);
        if (size > 0) { resize(size); }
    }

    inline ~flexlist()
    {
        m_pool.freepool().ptr<freelist_t>(m_freelist.load())->unref();
    }

    inline bool ready(typename freelist_t::ptr& fl, uint32_t size)
    {
        return fl && (fl->size() < size);
    }

    inline void* alloc(uint32_t size)
    {
        size = eco::align_up(size);
        typename freelist_t::ptr fl = freelist_get();
        // create new freelist that fit current size
        while (!ready(fl, size))
        {
            eco::spinlock lock(m_mutex);
            fl = freelist_get();
            if (!ready(fl, size))
            {
                if (!resize(size)) { return NULL; }
                fl = freelist_get();
            }
        }
        return fl->alloc();
    }

    inline void* realloc(void* obj, uint32_t size)
    {
        eco::lockfree::span* sp = m_pool.span_find(obj);
        size = eco::align_up(size);
        if (size <= sp->size()) { return obj; }
        
        // realloc from current freelist
        void* obj_new = alloc(size);
        if (obj_new != NULL)
        {
            memcpy(obj_new, obj, sp->size());
        }
        return obj_new;
    }

    inline void dealloc(void* obj)
    {
        eco::lockfree::span* span = m_pool.span_find(obj);
        typename freelist_t::ptr fl = freelist_get();
        if (fl->has(span))
        {
            fl->dealloc(obj);
        }
        else
        {
            span->unref();
        }
    }

private:
    inline eco::bool_t resize(uint32_t size)
    {
        // size *= 1.5
        size = size + (size >> 1);
        typename freelist_t::ptr fl = make_freelist<stack_t>(m_pool, size, 32);
        if (fl)
        {
            uint32_t fl_new = m_pool.freepool().offset(fl.get());
            uint32_t fl_old = m_freelist.exchange(fl_new);
            m_pool.freepool().ptr<freelist_t>(fl_old)->unref();
        }
        return fl != 0;
    }

    inline typename freelist_t::ptr freelist_get()
    {
        freelist_t* fl = m_pool.freepool().ptr<freelist_t>(m_freelist.load());
        return typename freelist_t::ptr(*fl);
    }

private:
    std::atomic<offset_t> m_freelist;
    spanpool&   m_pool;
    spinmutex   m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(lockfree);
eco_namespace_end(eco);