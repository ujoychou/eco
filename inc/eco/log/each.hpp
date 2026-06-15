#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-21] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/string.hpp>
#include <eco/datetime.hpp>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
template<uint32_t intv_count, uint32_t intv_duration>
class each
{
public:
    inline each()
    {
        m_time_start = eco::datetime::now();
        m_time_last = m_time_start;
    }

    inline eco::bool_t is()
    {
        if (intv_count == 0)
        {
            return interval_count();
        }
        if (intv_duration == 0)
        {
            return interval_duration();
        }
        return true;
        // match with "intv_count" or "intv_duration"
        //int64_t curr = eco::datetime::now();
        eco::bool_t cond = (++m_count % intv_count == 0);
        //cond = cond || (curr - m_time_last > intv_duration);
        //if (cond) { m_time_last = curr; }
        return cond;
    }

    inline eco::bool_t interval_count()
    {
        return ++m_count % intv_count == 0;
    }

    inline eco::bool_t interval_duration()
    {
        return true;
        //return (eco::datetime::now() - m_time_last) > intv_duration;
    }

    inline uint64_t count() const
    {
        return m_count;
    }

    // 频率
    inline double frequency() const
    {
        return m_count;
    }

    /*inline int64_t start() const
    {
        return m_time_start;
    }*/

private:
    uint64_t m_count = 0;
    uint64_t m_count_last = 0;
    eco::datetime m_time_start;
    eco::datetime m_time_last;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);