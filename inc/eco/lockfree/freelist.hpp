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
#include <eco/lockfree/spanpool.hpp>
#include <new>


eco_namespace(eco);
eco_namespace(lockfree);
class cachepool;
////////////////////////////////////////////////////////////////////////////////
template<typename stack_t = eco::lockfree::stack_mc>
class freelist : public stack_t
{
public:
    class ptr;
    
    inline freelist(eco::lockfree::cachepool& p, uint32_t size, uint32_t batch);

    inline freelist(eco::lockfree::spanpool& p, uint32_t size, uint32_t batch)
        : stack_t(p.freepool().base())
        , m_pool(p)
        , m_size(eco::align_up(size, 4))
        , m_batch(p.recount_batch(m_size, batch))
    {
        alloc_batch();
    }
    
    inline uint32_t size() const
    {
        return m_size;
    }

    inline uint32_t batch() const
    {
        return m_batch;
    }

    inline bool_t has(eco::lockfree::span* sp) const
    {
        return sp->size() == m_size && sp->owner() == this;
    }

    inline void* alloc()
    {
        void* obj = stack_t::pop();
        while (obj == NULL)
        {
            // alloc memory from cache pool.
            eco::spinlock lock(m_mutex);
            //printf("111111111111111111111111\n");
            obj = stack_t::pop();
            if (obj == NULL)
            {
                if (!alloc_batch()) { return NULL; }
                obj = stack_t::pop();
            }
        }
        return obj;
    }

    template<typename object_t>
    inline object_t* alloc()
    {
        return static_cast<object_t*>(alloc());
    }

    inline void dealloc(void* obj)
    {
        stack_t::push(obj);
    }

    inline bool local_mode() const
    {
        return m_ref.load() == 0;
    }

private:
    inline bool alloc_batch()
    {
        if (m_size > 0)
        {
            eco::lockfree::span* span = m_pool.span_alloc(m_size, m_batch, this);
            if (span != NULL)
            {
                span::range range(span->data(), 0);
                range.last = stack_t::link(span->data(), m_size * m_batch, m_size);
                stack_t::push(range.first, range.last);
                return true;
            }
        }
        return false;
    }

    inline bool dealloc_or_release(void* obj)
    {
        eco::lockfree::span* span = m_pool.span_find(obj);
        if (!has(span))
        {
            span->unref();
            return false;
        }
        dealloc(obj);
        return true;
    }

    inline void release()
    {
        eco::lockfree::span* sp = NULL;
        void* top = stack_t::release();
        while (top != NULL)
        {
            void* next = stack_t::next(top);
            sp = m_pool.span_find(top);
            sp->unref();
            top = next;
        }
    }

    inline void ref()
    {
        ++m_ref;
    }

    inline void unref()
    {
        uint32_t old = m_ref;
        while (old != 0 && !m_ref.compare_exchange_weak(old, old - 1)) {}

        // release freelist when unused
        if (old == 1)
        {
            release();
            m_pool.metapool().freelist_dealloc(this);
         }
    }

private:
    friend class ptr;
    eco::lockfree::spanpool&    m_pool;
    uint32_t                    m_size = 0;
    uint32_t                    m_batch = 0;
    std::atomic<uint32_t>       m_ref = 0;
    eco::spinmutex              m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
template<typename stack_t>
class freelist<stack_t>::ptr
{
public:
    typedef eco::lockfree::freelist<stack_t> freelist_t;
    explicit inline ptr(freelist_t& fl) : m_obj(&fl)
    {
        ref();
    }

    inline ptr(const freelist_t::ptr& fl) : m_obj(fl.m_obj)
    {
        ref();
    }

    inline ptr(freelist_t::ptr&& fl) : m_obj(fl.m_obj)
    {
        fl.m_obj = NULL;
    }

    inline freelist_t::ptr& operator=(
        const freelist_t::ptr& fl)
    {
        if (m_obj != fl.m_obj)
        {
            reset();
            m_obj = fl.m_obj;
            ref();
        }
        return *this;
    }

    inline freelist_t::ptr& operator=(freelist_t& fl)
    {
        if (m_obj != &fl)
        {
            reset();
            m_obj = &fl;
            ref();
        }
        return *this;
    }

    inline freelist_t::ptr& operator=(freelist::ptr&& fl)
    {
        reset();
        m_obj = fl.m_obj;
        fl.m_obj = NULL;
        return *this;
    }

    inline ~ptr()
    {
        reset();
    }

    inline freelist_t* operator->()
    {
        return m_obj;
    }

    inline const freelist_t* operator->() const
    {
        return m_obj;
    }

    inline freelist_t& operator*()
    {
        return *m_obj;
    }

    inline const freelist_t& operator*() const
    {
        return *m_obj;
    }

    inline freelist_t* get()
    {
        return m_obj;
    }

    inline const freelist_t* get() const
    {
        return m_obj;
    }

    inline operator eco::bool_t() const
    {
        return m_obj != NULL;
    }

    inline void reset()
    {
        if (m_obj) { m_obj->unref(); m_obj = NULL; }
    }

    inline freelist_t* release()
    {
        auto* tmp = m_obj;
        m_obj = NULL;
        return tmp;
    }

    inline uint32_t refc() const
    {
        return m_obj ? m_obj->m_ref.load() : 0u;
    }

private:
    inline void ref()
    {
        if (m_obj) { m_obj->ref(); }        
    }
    freelist_t* m_obj;
};


////////////////////////////////////////////////////////////////////////////////
template<typename stack_t = eco::lockfree::stack_mc>
inline typename eco::lockfree::freelist<stack_t>::ptr make_freelist(
    eco::lockfree::spanpool& p, uint32_t size, uint32_t batch)
{
    // alloc memory from metalist and freepool.
    auto* fl = static_cast<eco::lockfree::freelist<stack_t>*>(
        p.metapool().freelist_alloc());
    fl = new(fl) eco::lockfree::freelist<stack_t>(p, size, batch);
    return typename eco::lockfree::freelist<stack_t>::ptr(*fl);
}
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(lockfree);
eco_namespace_end(eco);