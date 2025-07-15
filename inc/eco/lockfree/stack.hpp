#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-04-25] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/macro.hpp>
#include <eco/type/number.hpp>
#include <atomic>
#include <stdlib.h>


eco_namespace(eco);
eco_namespace(lockfree);
////////////////////////////////////////////////////////////////////////////////
// unsafe stack: it need to be locked by multi-thread using.
class stack
{
private:
    struct node
    {
        node* next;
    };

    node* m_top;

public:
    inline stack(void* n = NULL) : m_top(NULL)
    {
        (void)n;
    }

    inline void* pop()
    {
        node* curr = m_top;
        if (m_top != NULL) { m_top = m_top->next; }
        return curr;
    }

    inline void push(void* n)
    {
        node* curr = static_cast<node*>(n);
        curr->next = m_top;
        m_top = curr;
    }

    inline void push(void* first, void* last)
    {
        static_cast<node*>(last)->next = m_top;
        m_top = static_cast<node*>(first);
    }

    inline void* release()
    {
        node* curr = m_top;
        m_top = NULL;
        return curr;
    }

    inline void* next(void* n)
    {
        return static_cast<node*>(n)->next;
    }

public:
    static inline void* link(char* first, uint32_t size, uint32_t obj)
    {
        node* n = NULL;
        size -= obj;
        for (uint32_t i = 0; i <= size; i += obj)
        {
            n  = reinterpret_cast<node*>(first + i);
            n->next = reinterpret_cast<node*>(first + i + obj);
        }
        if (n != NULL) { n->next = NULL; }
        return n;
    }
};


////////////////////////////////////////////////////////////////////////////////
// single-consumer stack
class stack_sc
{
protected:
    struct node
    {
        std::atomic<node*> next;
    };
    std::atomic<node*> m_top;

public:
    inline stack_sc(void* n = NULL) : m_top(NULL) 
    {
        (void)n;
    }

    inline void* pop()
    {
        // empty stack
        node* curr = m_top.load(std::memory_order_acquire);
        if (curr == NULL) { return NULL; }

        // single-consumer don't need to solve ABA problem
        node* next = curr->next.load(std::memory_order_acquire);
        while (!m_top.compare_exchange_weak(curr, next) && curr != NULL)
        {
            next = curr->next.load(std::memory_order_acquire);
        }
        return curr;
    }

    inline void push(void* n)
    {
        node* top_new = static_cast<node*>(n);
        node* top_old = m_top.load(std::memory_order_acquire);
        top_new->next.store(top_old, std::memory_order_release);
        while (!m_top.compare_exchange_weak(top_old, top_new))
        {
            top_new->next.store(top_old, std::memory_order_release);            
        }
    }

    inline void push(void* first, void* last)
    {
        node* node_first = static_cast<node*>(first);
        node* node_last = static_cast<node*>(last);
        node* top_old = m_top.load(std::memory_order_acquire);
        node_last->next.store(top_old, std::memory_order_release);
        while (!m_top.compare_exchange_weak(top_old, node_first))
        {
            node_last->next.store(top_old, std::memory_order_release);
        }
    }

    inline void* release()
    {
        node* top = m_top.load(std::memory_order_acquire);
        while (!m_top.compare_exchange_weak(top, NULL)) {}
        return top;
    }

    inline void* next(void* n)
    {
        return static_cast<node*>(n)->next.load(std::memory_order_acquire);
    }

    static inline void* link(char* first, uint32_t size, uint32_t obj)
    {
        node* n = NULL;
        size -= obj;
        for (uint32_t i = 0; i <= size; i += obj)
        {
            n = reinterpret_cast<node*>(first + i);
            n->next.store(
                reinterpret_cast<node*>(first + i + obj), 
                std::memory_order_release);
        }
        if (n != NULL) { n->next = NULL; }
        return n;
    }
};


////////////////////////////////////////////////////////////////////////////////
// multil-consumer stack
class stack_mc
{
private:
    struct top
    {
        eco::offset_t curr;     // current object address
        uint32_t aba;           // aba counter, solve aba problem

        inline top() {}
        inline top(uint32_t c, eco::offset_t n) : curr(c), aba(n) {}
    };
    const char*       m_base = 0; // base address of the stack
    std::atomic<top>  m_top;      // top object of the stack

protected:
    struct node
    {
        std::atomic<eco::offset_t> next;
    };

public:
    inline stack_mc(void* p) : m_base(static_cast<char*>(p)), m_top(top(0, 0))
    {}

    inline stack_mc(uint32_t object, uint32_t capacity) :  m_top(top(0, 0))
    {
        uint32_t size = object * capacity;
        char* first = static_cast<char*>(malloc(size));
        m_base = first - sizeof(void*);
        push(first, link(first, size, object));
    }

    inline void base(void* ptr)
    {
        m_base = static_cast<char*>(ptr);
    }

    inline void* ptr(eco::offset_t offset) const
    {
        return (offset != 0) ? const_cast<char*>(m_base + offset) : NULL;
    }

    template<typename object_t>
    inline object_t* ptr(eco::offset_t offset) const
    {
        return static_cast<object_t*>(ptr(offset));
    }

    inline eco::offset_t offset(void* node) const
    {
        return (static_cast<char*>(node) - m_base);
    }

    inline void* pop()
    {
        // empty stack
        top top_new;
        top top_old = m_top.load(std::memory_order_acquire);
        if (top_old.curr == 0) { return NULL; }
        
        do
        {
            node* n = ptr<node>(top_old.curr);
            top_new.curr = n->next.load(std::memory_order_acquire);
            top_new.aba =  top_old.aba;
        } while (!m_top.compare_exchange_weak(top_old, top_new) &&
                 top_old.curr != 0);
        return ptr<node>(top_old.curr);
    }

    template<typename object_t>
    inline object_t* pop()
    {
        return static_cast<object_t*>(pop());
    }

    inline void push(void* obj)
    {
        if (obj == NULL) { return; }
        node* n = static_cast<node*>(obj);

        top top_old = m_top.load(std::memory_order_acquire);
        top top_new(offset(obj), top_old.aba + 1);
        n->next.store(top_old.curr, std::memory_order_release);
        while (!m_top.compare_exchange_weak(top_old, top_new))
        {
            top_new.aba = top_old.aba + 1;
            n->next.store(top_old.curr, std::memory_order_release);
        }
    }

    inline void push(void* first, void* last)
    {
        node* node_lst = static_cast<node*>(last);
        top top_old = m_top.load(std::memory_order_acquire);
        top top_new(offset(first), top_old.aba);
        node_lst->next.store(top_old.curr, std::memory_order_release);
        while (!m_top.compare_exchange_weak(top_old, top_new))
        {
            top_new.aba = top_old.aba + 1;
            node_lst->next.store(top_old.curr, std::memory_order_release);
        }
    }

    inline void push(eco::offset_t first, eco::offset_t last)
    {
        push(ptr(first), ptr(last));
    }

    inline void* release()
    {
        top top_new(0, 0);
        top top_old = m_top.load(std::memory_order_acquire);
        while (!m_top.compare_exchange_weak(top_old, top_new)) {}
        return ptr(top_old.curr);
    }

    inline void* next(void* n)
    {
        return ptr(static_cast<node*>(n)->next.load(std::memory_order_acquire));
    }

    static inline void* link(
        char* first, eco::offset_t first_pos, uint32_t size, uint32_t obj)
    {
        node* n = NULL;
        size -= obj;
        for (uint32_t i = 0; i <= size; i += obj)
        {
            n = reinterpret_cast<node*>(first + i);
            n->next.store(first_pos + i + obj, std::memory_order_release);
        }
        if (n != NULL) { n->next = 0; }
        return n;
    }

    inline void* link(char* first, uint32_t size, uint32_t obj)
    {
        return link(first, offset(first), size, obj);
    }
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(lockfree);
eco_namespace_end(eco);
