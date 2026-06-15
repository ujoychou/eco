#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-09-04] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/string.hpp>
#include <eco/string/string_entry.hpp>
#include <eco/export/api.hpp>
#include <eco/log/message.hpp>
#include <stdarg.h>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
struct except_input
{
    int level;
    int id;
    int line;
    const char* file;
    const char* func;
};

struct except_impl
{
    // except id: "service + kind + id = trade/9031001"
    // except kind: using for log and user prompt.
    int         id;         // except id: "kind(8bit) + id(24bit)".
    // const char* service; // the name of service that client request.
    const char* args;       // except context arguments data, used by caller.

    // data that serialized bytes.
    eco::string_entry entry_args;

    // except logs
    eco::log::message logs;
    
    inline except_impl(int le, int id, int li, const char* fi, const char* fc)
        : id(id), args(0), logs(le, li, fi, fc)
    {}
};

class eco_api except_api
{
public:
    static inline eco::except_impl* this_except();
    static inline eco::except_impl* this_except(const except_input& i);

    static inline void this_except_log(const eco::except_impl& e);
};


////////////////////////////////////////////////////////////////////////////////
template<typename actual_t>
class except_t : public eco::stream_t<actual_t>
{
protected:
    eco::except_impl* m_impl = 0;
    inline actual_t& rthis() { return (actual_t&)(*this); }

public:
    inline ~except_t()
    {
        if (has_logs())
        {
            except_api::this_except_log(*m_impl);
        }
    }

    inline int id() const
    {
        return m_impl->id;
    }

    template<typename args_t>
    inline actual_t& args(args_t v)
    {
        //eco::c_str str(v);
        //m_impl->entry_args.append(str.value(), str.size());
        return rthis();
    }

    inline const char* args() const
    {
        return m_impl->entry_args.c_str();
    }

    inline actual_t& mode(int v)
    {
        m_impl->logs.level = eco::log::level_mode(m_impl->logs.level, v);
        return rthis();
    }

    inline actual_t& user(const char* v)
    {
        m_impl->logs.user = v;
        return rthis();
    }

    inline actual_t& aspect(const char* v)
    {
        m_impl->logs.aspect = v;
        return rthis();
    }

    inline const char* what() const
    {
        return m_impl->logs.entry.c_str();
    }

    inline bool has_logs() const
    {
        return !m_impl->logs.entry.null();
    }

    inline actual_t& append(char c, uint32_t size)
    {
        if (has_logs())
        {
            m_impl->logs.entry.append(c, size);
        }
        return rthis();
    }

    inline actual_t& append(const char* str, uint32_t size)
    {
        if (has_logs())
        {
            m_impl->logs.entry.append(str, size);
        }
        return rthis();
    }

    inline actual_t& printf(const char* format, ...)
    {
        if (has_logs())
        {
            va_list args;
            va_start(args, format);
            //m_logs.entry.printf(format, args);
            va_end(args);
        }
        return rthis();
    }
    inline actual_t& format(const char* format, ...)
    {
        if (has_logs())
        {
            va_list args;
            va_start(args, format);
            //m_logs.entry.printf(format, args);
            va_end(args);
        }
        return rthis();
    }
};


////////////////////////////////////////////////////////////////////////////////
// eco exception that can return <type_t> value.
template<typename type_t>
class except_return : public except_t<except_return<type_t>>
{
public:
    inline except_return(
        int level, int id, int line, const char* file, const char* func, type_t v)
    {
        except_input input{level, id, line, file, func};
        this->m_impl = except_api::this_except(input);
        m_return = v;
    }

    inline operator type_t() const
    {
        return m_return;
    }

private:
    type_t m_return;
};

// eco exception that can return <type_t> value.
template <typename type_t>
inline except_return<type_t> except_r(
    int level, int id, int line, const char* file, const char* func, type_t v)
{
    return except_return<type_t>(level, id, line, file, func, v);
}


////////////////////////////////////////////////////////////////////////////////
// eco exception.
class except : public except_t<except>
{
public:
    inline except()
    {
        m_impl = except_api::this_except();
    }

    inline except(
        int level, int id, int line, const char* file, const char* func)
    {
        except_input input{level, id, line, file, func};
        m_impl = except_api::this_except(input);
    }

    inline operator bool() const
    {
        return m_impl->id != 0;
    }

    static inline except this_except()
    {
        return except();
    }
};


////////////////////////////////////////////////////////////////////////////////
// return except mode: set except information in sub function
#define eco_except_2(level, id) \
eco::except(eco::log::level, id, __LINE__, __FILE__, __func__)
// return except mode: return "false" when condition is true.
#define eco_except_3(level, id, when) \
for (int w = 1; w && (when); w = 0) \
    return eco::except(eco::log::level, id, __LINE__, __FILE__, __func__)
// return except mode: return "value" when condition is true.
#define eco_except_4(level, id, when, value) \
for (int w = 1; w && (when); w = 0) \
    return eco::except_r(eco::log::level, id, __LINE__, __FILE__, __func__, value)
// return except mode:
#define eco_except(...) eco_macro_overload(eco_except_,__VA_ARGS__)

// throw except mode: throw except reading from config.
#define eco_throw_2(level, id) \
throw eco::except(eco::log::level, id, __LINE__, __FILE__, __func__)
// throw except mode: throw except when condition is true.
#define eco_throw_3(level, id, when) \
for (int w = 1; w && (when); w = 0) \
    throw eco::except(eco::log::level, id, __LINE__, __FILE__, __func__)
// throw except mode:
#define eco_throw(...) eco_macro_overload(eco_throw_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);