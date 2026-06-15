/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-08-13] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/log.hpp>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
class sink : public eco::rtti::object
{
    eco_rtti_interface(sink);
public:
    virtual ~sink() {}
    virtual void output(eco::log::message& msg) = 0;
};


////////////////////////////////////////////////////////////////////////////////
class sink_file : public eco::log::sink
{
    eco_rtti(sink_file, eco::log::sink, "file");
public:
    virtual void output(eco::log::message& msg) override
    {
        // call eco::os::write_console();
        std::cout << msg.entry.text() << std::endl;
    }
};


////////////////////////////////////////////////////////////////////////////////
class sink_console : public eco::log::sink
{
    eco_rtti(sink_console, eco::log::sink, "console");
public:
    virtual void output(eco::log::message& msg) override
    {
        // call eco::os::write_file();
        // call eco::os::write_mmap(), when high throughput.
        fwrite(msg.entry.data(), msg.entry.size(), 1, m_file);
        fflush(m_file);
    }

private:
    FILE* m_file = 0;
    char* m_path = 0;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);