#include <eco/plugin.hpp>
////////////////////////////////////////////////////////////////////////////////
#include <map>
#include <vector>
#include <string>


eco_namespace(eco);
eco_namespace(rtti);
template<typename value_t>
struct std_vector_binary_map
{
    std::vector<value_t> datas;

    void* get(const char* key)
    {
        return nullptr;
    }
};
////////////////////////////////////////////////////////////////////////////////
struct plugin_registry_impl
{
    std::map<std::string, void*> object_map;

    // order by: plugin name and version
    // exp: a.1.2.1, a.1.11.1; b.1.0.1, b.1.0.2, b.1.0.12; c.0.1.1
	eco::std_vector_binary_map<eco::rtti::plugin_type*> type_map;
} g_impl;


////////////////////////////////////////////////////////////////////////////////
void plugin_registry::set(eco::rtti::plugin_type* plugin)
{
    std::vector<eco::rtti::plugin_type*> datas;

    std::lower_bound(datas.begin(), datas.end(), plugin, [](const eco::rtti::plugin_type* a, const eco::rtti::plugin_type* b) {
    for (auto it = g_impl.type_map.datas.begin(); it != g_impl.type_map.datas.end(); ++it)
    {
        if (it->m_name == plugin->m_name)
        {
            datas.push_back(plugin);
        }
    }
    g_impl.type_map.datas.push_back(plugin);
} 
eco::plugin_type* plugin_registry::get(
    const char* name,
    const char* version)
{
    auto it = g_impl.type_map.find(name);
    return (it != g_impl.type_map.end()) ? it->second : nullptr;
}
eco::plugin_type* plugin_registry::get_compatible(
    const char* name,
    const char* version)
{
    auto it = g_impl.type_map.find(name);
    if (it != g_impl.type_map.end())
    {
        if (it->second->m_compatible(version))
        {
            return it->second;
        }
    }
    return nullptr;
}


////////////////////////////////////////////////////////////////////////////////
void* plugin::get_object(
    const char* plugin_name, 
    const char* plugin_version,
    const char* object_name)
{
    // exist object
    auto it = g_impl.object_map.find(object_name);
    if (it != g_impl.object_map.end())
    {
        return it->second;
    }

    // create new object
    auto* type = get_plugin_compatible(plugin_name, plugin_version);
    if (type == nullptr)
    {
        eco_throw(eco::error::invalid_argument, "plugin not found");
        return nullptr;
    }
    void* object = type->m_create(object_name);
    if (object == nullptr)
    {
        eco_throw(eco::error::invalid_argument, "object not found");
        return nullptr;
    }
    g_impl.object_map[object_name] = object;
    return object;
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rtti);
eco_namespace_end(eco);