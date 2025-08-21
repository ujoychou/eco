#include <eco/plugin.hpp>
////////////////////////////////////////////////////////////////////////////////
#include <eco/type.hpp>
#include <map>



eco_namespace(eco);
eco_namespace(rtti);
////////////////////////////////////////////////////////////////////////////////
struct plugin_type_impl : public plugin_type
{
    eco_object(plugin_type_impl);
public:
    struct identity
    {
        const char* name;
        const char* version;
    };

    inline plugin_type_impl(
        const char* name,
        const char* version,
        const char* dllpath,
        plugin_type::create_t create)
        : plugin_type(name, version, dllpath, create)
    {}

public:
    eco::rtti::dll dll;
};


////////////////////////////////////////////////////////////////////////////////
struct plugin_type_compare
{
    inline int operator()(
        const plugin_type_impl::ptr& a,
        const plugin_type_impl::ptr& b)
    {
        int cmp = strcmp(a->name.c_str(), b->name.c_str());
        if (cmp < 0) { return -1; }
        if (cmp > 0) { return 1; }
        return eco::compare_version(a->version.c_str(), b->version.c_str());
    }

    inline int operator()(
        const plugin_type_impl::ptr& a,
        const plugin_type_impl::identity& b)
    {
        int cmp = strcmp(a->name.c_str(), b.name);
        if (cmp < 0) { return -1; }
        if (cmp > 0) { return 1; }
        return eco::compare_version(a->version.c_str(), b.version);
    }
};


////////////////////////////////////////////////////////////////////////////////
struct plugin_registry_impl
{
    std::map<std::string, void*> object_map;

    // order by: plugin name and version
    // exp: a.1.2.1 < a.1.11.1 < b.1.0.2 < b.1.0.12 < c.0.1.1
    eco::vector_map<plugin_type_impl::ptr, plugin_type_compare> type_map;

public:
    inline plugin_type_impl::ptr get_plugin(
        const char* name, const char* version)
    {
        plugin_type_impl::identity id{name, version};
        plugin_type_impl::ptr* it = type_map.find(id);
        return it ? *it : nullptr;
    }

    inline plugin_type_impl::ptr get_plugin_compatible(
        const char* name, const char* version)
    {
        plugin_type_impl::identity id{name, version};
        auto it = type_map.lower_bound(id);
        if (it == type_map.data().end())
        {
            return nullptr;
        }
        return ((**it).name == name) ? (*it) : nullptr;
    }
};
static plugin_registry_impl g_impl;


////////////////////////////////////////////////////////////////////////////////
eco::rtti::plugin_type::ptr plugin_registry::set_plugin(
    const char* name,
	const char* version,
	const char* dllpath,
	plugin_type::create_t create)
{
    plugin_type_impl::ptr info = std::make_shared<plugin_type_impl>(
        name, version, dllpath, create);
    plugin_type_impl::ptr* it = g_impl.type_map.set(info);
    return it ? *it : nullptr;
} 

eco::rtti::plugin_type::ptr plugin_registry::get_plugin(
    const char* name,
    const char* version)
{
    return g_impl.get_plugin(name, version);
}

eco::rtti::plugin_type::ptr plugin_registry::get_plugin_compatible(
    const char* name,
    const char* version)
{
    return g_impl.get_plugin_compatible(name, version);
}


////////////////////////////////////////////////////////////////////////////////
void* plugin::get(const char* name, const char* version, const char* object_name)
{
    // exist object
    auto it = g_impl.object_map.find(object_name);
    if (it != g_impl.object_map.end())
    {
        return it->second;
    }

    // create new object
    plugin_type_impl::ptr type = g_impl.get_plugin_compatible(name, version);
    if (type == nullptr)
    {
        //eco_throw(eco::error::invalid_argument, "plugin not found");
        return nullptr;
    }
    if (!type->dll) // reload dll
    {
        type->dll.load(type->dllpath.c_str());
        //eco_throw(type->dll, "plugin load failed");
        type = g_impl.get_plugin_compatible(name, version);
    }
    void* object = type->create();
    if (object == nullptr)
    {
        //eco_throw(eco::error::invalid_argument, "object not found");
        return nullptr;
    }
    g_impl.object_map[object_name] = object;
    return object;
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rtti);
eco_namespace_end(eco);