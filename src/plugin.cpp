#include <eco/plugin.hpp>
////////////////////////////////////////////////////////////////////////////////
#include <eco/type.hpp>
#include <map>



eco_namespace(eco);
eco_namespace(rtti);
////////////////////////////////////////////////////////////////////////////////
struct plugin_type_view
{
	const char* name;
    const char* version;
    plugin_type::create_t create;
};
struct plugin_type_impl : public plugin_type
{
	eco::rtti::dll  dll;

    inline bool operator==(const plugin_type_view& v) const
    {
        return name == v.name && version == v.version;
    }

    explicit inline plugin_type_impl(const plugin_type_view& v)
    {
        plugin_type::name = v.name;
        plugin_type::version = v.version;
        plugin_type::create = v.create;
    }

    inline plugin_type_impl& operator=(const plugin_type_view& v)
    {
        plugin_type::name = v.name;
        plugin_type::version = v.version;
        plugin_type::create = v.create;
        return *this;
    }
};
struct plugin_type_compare
{
    template<typename plubin_type_id>
    inline bool operator()(const plugin_type_impl& a, const plubin_type_id& b)
    {
        int cmp = strcmp(a.name.c_str(), b.name);
        if (cmp < 0) { return true; }
        if (cmp > 0) { return false; }
        return eco::compare_version(a.version.c_str(), b.version) < 0;
    }
};
struct plugin_registry_impl
{
    std::map<std::string, void*> object_map;

    // order by: plugin name and version
    // exp: a.1.2.1 < a.1.11.1 < b.1.0.2 < b.1.0.12 < c.0.1.1
    eco::vector_map<plugin_type_impl, plugin_type_compare> type_map;
};
static plugin_registry_impl g_impl;


////////////////////////////////////////////////////////////////////////////////
eco::rtti::plugin_type* plugin_registry::set_plugin(
    const char* name,
	const char* version,
	eco::rtti::plugin_type::create_t create)
{
    plugin_type_view view{name, version, create};
    return g_impl.type_map.set(view);
} 

eco::rtti::plugin_type* plugin_registry::get_plugin(
    const char* name,
    const char* version)
{
    plugin_type_view view{name, version, 0};
    return g_impl.type_map.find(view);
}

eco::rtti::plugin_type* plugin_registry::get_plugin_compatible(
    const char* name,
    const char* version)
{
    plugin_type_view view{name, version, 0};
    auto it = g_impl.type_map.lower_bound(view);
    return it->name == name ? &(*it) : nullptr;
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
    plugin_type_impl* type = static_cast<plugin_type_impl*>(
        plugin_registry::get_plugin_compatible(name, version));
    if (type == nullptr)
    {
        //eco_throw(eco::error::invalid_argument, "plugin not found");
        return nullptr;
    }
    if (!type->dll) // reload dll
    {
        type->dll.load(type->dllpath.c_str());
        //eco_throw(type->dll, "plugin load failed");
    }
    void* object = type->create(object_name);
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