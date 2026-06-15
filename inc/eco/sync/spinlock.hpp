#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-06-10] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2025 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/prec.hpp>
#include <atomic>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
class spinmutex
{
public:
    inline void lock()
    {
        while (m_flag.test_and_set(std::memory_order_acquire))
        {
            cpu_pause();
        }
    }

    inline void unlock() 
    {
        m_flag.clear(std::memory_order_release);
    }

private:
    inline void cpu_pause()
    {}

    std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};


////////////////////////////////////////////////////////////////////////////////
class spinlock
{
public:
    inline spinlock(spinmutex& mutex) : m_mutex(mutex)
    {
        m_mutex.lock();
    }

    inline ~spinlock() 
    {
        m_mutex.unlock();
    }

private:
    spinmutex& m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);