#include <eco/cache.hpp>
////////////////////////////////////////////////////////////////////////////////
#include <eco/lockfree/stack.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
struct cache_impl
{
public:
    uint32_t logs_entry_size = 0;
    eco::lockfree::stack_mc cache_logs;
};
struct cache_error_this
{
    char* buffer;
    eco::bool_t nolog;
};


static eco::cache_impl s_impl;
static thread_local cache_error_this this_cache_error;
static thread_local eco::lockfree::stack this_cache_logs;
////////////////////////////////////////////////////////////////////////////////
eco::entry& cache::entry_this_error()
{
    if (this_cache_error.buffer == nullptr)
    {
        this_cache_error.buffer = static_cast<char*>(
            s_impl.cache_logs.pop());
    }
    return this_cache_error;
}


////////////////////////////////////////////////////////////////////////////////
eco::entry cache::entry_this_borrow()
{
    eco::entry entry;
    uint32_t capacity = s_impl.logs_entry_size;
    entry.reset(static_cast<char*>(this_cache_logs.pop()), capacity);
    if (entry.text() == nullptr)
    {
        entry.reset(static_cast<char*>(s_impl.cache_logs.pop()), capacity);
    }
    return entry;
}
////////////////////////////////////////////////////////////////////////////////
void cache::entry_this_return(eco::entry& entry)
{
    this_cache_logs.push(const_cast<char*>(entry.text()));
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);