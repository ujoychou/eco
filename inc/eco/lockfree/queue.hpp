#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-06-03] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2025 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/lockfree/stack.hpp>
#include <atomic>


eco_namespace(eco);
eco_namespace(lockfree);
////////////////////////////////////////////////////////////////////////////////
class fifo_mpmc
{
private:
    struct pointer
    {
        eco::offset_t ptr = 0;
        uint32_t aba = 0;

        inline pointer(eco::offset_t o, uint32_t c) : ptr(o), aba(c) {}
    };

    struct node
    {
        std::atomic<pointer> next;
        void* value = 0;
    };
    
public:
    inline fifo_mpmc(uint32_t capacity) 
        : m_freelist(sizeof(node), capacity + 1)
        , m_head(pointer(0, 0))
        , m_tail(pointer(0, 0))
    {
        node* dummy = m_freelist.pop<node>();
        dummy->next = pointer(0, 0);
        dummy->value = NULL;
        pointer node_ptr(m_freelist.offset(dummy), 0);
        m_head = node_ptr;
        m_tail = node_ptr;
    }

    inline bool empty() const
    {
        pointer head = m_head;
        node*   head_node = m_freelist.ptr<node>(head.ptr);
        pointer next = head_node->next;
        return next.ptr == 0;
    }

    inline void* pop_front()
    {
        while (true)
        {
            pointer head = m_head;
            pointer tail = m_tail;
            node*   head_node = m_freelist.ptr<node>(head.ptr);
            pointer next = head_node->next;

            // empty queue
            if (next.ptr == 0) { break; }

            // tail is not updated
            if (head.ptr == tail.ptr)
            {
                node*   tail_node = m_freelist.ptr<node>(tail.ptr);
                pointer tail_next(tail_node->next.load().ptr, tail.aba + 1);
                m_tail.compare_exchange_weak(tail, tail_next);
            }
            // pop value from head
            else
            {
                node* next_node = m_freelist.ptr<node>(next.ptr);
                void* value = next_node->value;
                pointer head_next(next.ptr, head.aba + 1);
                if (m_head.compare_exchange_weak(head, head_next))
                {
                    m_freelist.push(head_node);
                    return value;
                }
            }
        }
        return NULL;
    }

    template<typename object_t>
    inline object_t* pop_front()
    {
        return static_cast<object_t*>(pop_front());
    }

    inline eco::bool_t push_back(void* obj)
    {
        node* new_node = m_freelist.pop<node>();
        if (new_node == NULL) { return false; }
        new_node->next = pointer(0, 0);
        new_node->value = obj;
        pointer new_ptr(m_freelist.offset(new_node), 0);

        while (true)
        {
            pointer tail = m_tail;
            node*   tail_node = m_freelist.ptr<node>(tail.ptr);
            pointer next = tail_node->next;
            if (next.ptr == 0)
            {
                new_ptr.aba = next.aba + 1;
                if (tail_node->next.compare_exchange_weak(next, new_ptr))
                {
                    new_ptr.aba = tail.aba = 1;
                    m_tail.compare_exchange_weak(tail, new_ptr);
                    break;
                }
            }
            else
            {
                next.aba = tail.aba + 1;
                m_tail.compare_exchange_weak(tail, next);
            }
        }
        return true;
    }

private:
    stack_mc m_freelist;
    std::atomic<pointer> m_head;
    std::atomic<pointer> m_tail;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(lockfree);
eco_namespace_end(eco);