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
#include <eco/lockfree/stack.hpp>
#include <eco/sync/spinlock.hpp>


eco_namespace(eco);
eco_namespace(lockfree);
class span;
class page_radix_it;
////////////////////////////////////////////////////////////////////////////////
struct freepolicy
{
private:
    static const uint32_t PAGE_BITS = 12;           // 4K
    static const uint32_t BUFF_SIZE = (1 << 23);    // 8M
    uint32_t m_memory_size  = BUFF_SIZE;
    uint32_t m_page_bits    = PAGE_BITS;

public:
    inline eco::lockfree::freepolicy& memory_size(uint32_t memory_size)
    {
        m_memory_size = memory_size;
        if (m_memory_size < this->page_size())
            m_memory_size = this->page_size();
        else
            m_memory_size = align_up(m_memory_size, this->page_size());
        return *this;
    }

    inline uint32_t memory_size()
    {
        return m_memory_size;
    }

    inline uint32_t page_size() const
    {
        return (1 << m_page_bits);
    }

    inline uint32_t size_to_page(uint32_t size) const
    {
        uint32_t page = size >> m_page_bits;
        if (size - (page << m_page_bits) > 0) { page += 1; }
        return page;
    }

    inline uint32_t page_to_size(uint32_t page) const
    {
        return page << m_page_bits;
    }

    inline uint32_t pageid(offset_t offset) const
    {
        return offset >> m_page_bits;
    }

    inline uint32_t round_page_size(uint32_t size) const
    {
        return page_to_size(size_to_page(size)); 
    }
};


////////////////////////////////////////////////////////////////////////////////
class freepool
{
public:
    inline freepool() : m_curr(0)
    {}

    inline ~freepool()
    {
        if (m_data)
        {
            free(base_actual());
        }
    }

    inline bool ready() const
    {
        return m_data != NULL;
    }

    inline bool init(eco::lockfree::freepolicy& policy)
    {
        if (m_data != NULL) { return false; }
        
        m_size = policy.memory_size();
        m_data = static_cast<char*>(malloc(m_size));
        if (m_data == NULL) { return false; }

        // adjust base address, for relative offset aways >= "1 page size"
        m_curr  = m_policy.page_size();
        m_size += m_policy.page_size();
        m_data -= m_policy.page_size();
        return true;
    }

    inline const eco::lockfree::freepolicy& policy() const
    {
        return m_policy;
    }

    inline char* base() const
    {
        return m_data;
    }

    inline char* base_actual() const
    {
        return m_data + m_policy.page_size();
    }

    inline char* ptr(uint32_t offset) const
    {
        return offset != 0 ? (m_data + offset) : NULL;
    }

    template<typename object_t>
    inline object_t* ptr(uint32_t offset)
    {
        return reinterpret_cast<object_t*>(ptr(offset));
    }

    inline uint32_t offset(void* ptr) const
    {
        return ptr ? (uint32_t)((char*)(ptr) - m_data) : 0;
    }

    inline uint32_t pageid(void* ptr) const
    {
        return m_policy.pageid(offset(ptr));
    }

    inline uint32_t alloc(uint32_t size)
    {
        uint32_t curr_old = m_curr.load(std::memory_order_acquire);
        uint32_t curr_new;
        do
        {
            curr_new = curr_old + size;
            if (curr_new > m_size) { return 0; }     // the memory is out
        } while (!m_curr.compare_exchange_weak(curr_old, curr_new));
        return curr_old;
    }

    inline char* alloc_ptr(uint32_t size)
    {
        return ptr(alloc(size));
    }
    
private:
    char*       m_data = 0;
    uint32_t    m_size = 0;
    std::atomic<uint32_t> m_curr;
    eco::lockfree::freepolicy m_policy;
};


////////////////////////////////////////////////////////////////////////////////
class metapool
{
public:
    inline metapool(
        eco::lockfree::freepool& freepool, 
        uint32_t spanmeta, 
        uint32_t freelist, 
        uint32_t radix_it)
        : m_spanmeta_size(align_up(spanmeta))
        , m_freelist_size(align_up(freelist))
        , m_radix_it_size(align_up(radix_it))
        , m_freepool(freepool)
        , m_spanmeta_list(NULL)
        , m_freelist_list(NULL)
        , m_radix_it_list(NULL)
    {
        init(m_spanmeta_list, m_spanmeta_size);
    }

    inline bool init(eco::lockfree::freepolicy& policy)
    {
        if (m_freepool.init(policy))
        {
            m_spanmeta_list.base(m_freepool.base());
            m_freelist_list.base(m_freepool.base());
            m_radix_it_list.base(m_freepool.base());
            return true;            
        }
        return false;
    }

    inline eco::lockfree::freepool& freepool()
    {
        return m_freepool;
    }

    inline void* span_alloc()
    {
        return alloc<void>(m_spanmeta_list, m_spanmeta_mutex, m_spanmeta_size);
    }

    inline void span_dealloc(void* sp)
    {
        m_spanmeta_list.push(sp);
    }

    inline void* freelist_alloc()
    {
        return alloc<void>(m_freelist_list, m_freelist_mutex, m_freelist_size);
    }

    inline void freelist_dealloc(void* obj)
    {
        m_freelist_list.push(obj);
    }

    inline void* radix_it_alloc()
    {
        void* it = alloc<void>(m_radix_it_list, m_radix_it_mutex, m_radix_it_size);
        // [TODO DELETE]
        //memset(it, 0, m_radix_it_size);
        return it;
    }

    inline void radix_dealloc(void* obj)
    {
        m_radix_it_list.push(obj);
    }

private:
    inline bool init(eco::lockfree::stack_mc& mc, uint32_t size)
    {
        uint32_t total = m_freepool.policy().round_page_size(size * 64);
        char* first = m_freepool.alloc_ptr(total);
        if (first != NULL)
        {
            void* last = mc.link(first, m_freepool.offset(first), total, size);
            mc.push(first, last);
        }
        return first != 0;
    }

    template<typename object_t>
    inline object_t* alloc(
        eco::lockfree::stack_mc& mc, eco::spinmutex& mutex, uint32_t size)
    {
        void* obj = mc.pop();
        while (obj == NULL)
        {
            // alloc memory from metapool and freepool.
            spinlock lock(mutex);
            obj = mc.pop();
            if (obj == NULL)
            {
                if (!init(mc, size)) { break; }
                obj = mc.pop();
            }
        }
        return static_cast<object_t*>(obj);
    }
    
private:
    uint32_t  m_spanmeta_size;
    uint32_t  m_freelist_size;
    uint32_t  m_radix_it_size;
    eco::lockfree::freepool& m_freepool;
    eco::lockfree::stack_mc  m_spanmeta_list;
    eco::lockfree::stack_mc  m_freelist_list;
    eco::lockfree::stack_mc  m_radix_it_list;
    eco::spinmutex m_spanmeta_mutex;
    eco::spinmutex m_freelist_mutex;
    eco::spinmutex m_radix_it_mutex;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(lockfree);
eco_namespace_end(eco);