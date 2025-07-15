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
#include <eco/lockfree/page_radix.hpp>


eco_namespace(eco);
eco_namespace(lockfree);
class spanlist;
////////////////////////////////////////////////////////////////////////////////
class span
{
public:
    struct range
    {
        void* first;
        void* last;
        inline range(void* f, void* l) : first(f), last(l) {}
    };

    inline void init(eco::lockfree::spanlist& list, char* data)
    {
        m_list = &list;
        m_data = data;
    }

    inline uint32_t pages() const;

    inline eco::lockfree::spanlist* spanlist() const
    {
        return m_list;
    }

    inline char* data() const
    {
        return m_data;
    }

public:
    inline void* owner() const
    {
        return m_owner;
    }

    inline void owner(void* owner, uint32_t size, uint32_t batch)
    {
        m_owner = owner;
        m_size = size;
        m_ref = batch;
    }

    inline uint32_t size() const
    {
        return m_size;
    }

    inline uint32_t refc() const
    {
        return m_ref;
    }

    inline void ref()
    {
        ++m_ref;
    }

    inline bool unref();

private:
    // freelist
    void*       m_owner = 0;
    uint32_t    m_size = 0;         // object size of freelist
    std::atomic<uint32_t> m_ref;    // object ref count

    // spanlist
    eco::lockfree::spanlist* m_list;// spanlist

    // m_data can't be the 1st member of span. because freelist 
    // node_t.next will overwrite it.
    char*       m_data = 0;         // span memory address
};


////////////////////////////////////////////////////////////////////////////////
class spanlist : public eco::lockfree::stack_mc
{
public:
    inline spanlist() : eco::lockfree::stack_mc(NULL)
    {}

    inline void init(char* base, uint32_t index, uint32_t pages)
    {
        m_index = index;
        m_pages = pages;
        eco::lockfree::stack_mc::base(base);
    }

    inline eco::lockfree::span* alloc()
    {
        return static_cast<eco::lockfree::span*>(eco::lockfree::stack_mc::pop());
    }

    inline eco::lockfree::span* alloc_from_freepool(eco::lockfree::metapool& meta)
    {
        // alloc memory from metapool and freepool.
        eco::lockfree::freepool& pool = meta.freepool();
        auto span = static_cast<eco::lockfree::span*>(meta.span_alloc());
        span->init(*this, pool.alloc_ptr(pool.policy().page_to_size(m_pages)));
        if (span->data() == NULL)
        {
            meta.span_dealloc(span);
            span = NULL;
        }
        return span;
    }

    inline void dealloc(eco::lockfree::span* span)
    {
        eco::lockfree::stack_mc::push(span);
    }

    inline uint32_t pages() const
    {
        return m_pages;
    }

    inline uint32_t index() const
    {
        return m_index;
    }

private:
    uint32_t m_pages;
    uint32_t m_index;
};


////////////////////////////////////////////////////////////////////////////////
class spanpool
{
public:
    inline spanpool(
        eco::lockfree::freepool& freepool,
        eco::lockfree::metapool& metapool)
        : m_freepool(freepool)
        , m_metapool(metapool)
    {
        // normal pages: 1page ~ 64page = 4k ~ 256k / 8k ~ 512k
        for (uint32_t i = 0; i < SPAN_NUM - 1; i++)
        {
            uint32_t pages = 1 << i;
            m_spanlist[i].init(NULL, i, pages);
        }
        // huge pages: 512page = 2M / 4M
        m_spanlist[SPAN_NUM - 1].init(NULL, SPAN_NUM - 1, 512);
    }

    inline bool init(eco::lockfree::freepolicy& policy)
    {
        if (m_metapool.init(policy))
        {
            for (uint32_t i = 0; i < SPAN_NUM; i++)
            {
                m_spanlist[i].base(m_freepool.base());
            }
            return true;
        }
        return false;
    }

    inline eco::lockfree::metapool& metapool()
    {
        return m_metapool;
    }

    inline eco::lockfree::freepool& freepool()
    {
        return m_freepool;
    }

    inline eco::lockfree::spanlist* spanlist_hugepage()
    {
        return &m_spanlist[SPAN_NUM - 1];
    }

    inline eco::lockfree::spanlist* spanlist_find(uint32_t pages)
    {
        // normal pages span in span array
        uint32_t normal_num = SPAN_NUM - 1;
        for (uint32_t i = 0; i < normal_num; i++)
        {
            if (pages <= m_spanlist[i].pages()) { return &m_spanlist[i]; }
        }
        return spanlist_hugepage();
    }

    inline eco::lockfree::span* span_find(void* obj)
    {
        return m_radix.find(m_freepool.pageid(obj));
    }

    inline void span_index_setup(eco::lockfree::span& span)
    {
        uint32_t page = m_freepool.pageid(span.data());
        for (uint32_t i = 0; i < span.pages(); ++i, ++page)
        {
            m_radix.set(page, &span, m_metapool);
        }   
    }

    inline eco::lockfree::span* span_alloc(
        uint32_t size, uint32_t batch, void* owner)
    {
        uint32_t pages = m_freepool.policy().size_to_page(size * batch);
        eco::lockfree::spanlist* splist = spanlist_find(pages);
        eco::lockfree::span* span = splist->alloc();
        if (span == NULL)
        {
            span = splist->alloc_from_freepool(m_metapool);
            if (span == NULL) { return NULL; }
            span_index_setup(*span);
        }
        span->owner(owner, size, batch);
        return span;
    }

    inline uint32_t recount_batch(uint32_t size, uint32_t batch)
    {
        uint32_t page = m_freepool.policy().size_to_page(size * batch);
        eco::lockfree::spanlist* splist = spanlist_find(page);
        return m_freepool.policy().page_to_size(splist->pages()) / size;
    }
    
private:
    static const uint32_t SPAN_NUM = 8;
    eco::lockfree::freepool& m_freepool;
    eco::lockfree::metapool& m_metapool;
    eco::lockfree::spanlist  m_spanlist[SPAN_NUM];
    eco::lockfree::page_radix_32 m_radix;
};


////////////////////////////////////////////////////////////////////////////////
inline bool eco::lockfree::span::unref()
{
    if (--m_ref == 0)
    {
        m_list->dealloc(this);
        return true;
    }
    return false;
}
inline uint32_t eco::lockfree::span::pages() const
{
    return m_list->pages();
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(lockfree);
eco_namespace_end(eco);