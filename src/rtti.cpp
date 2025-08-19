#include <eco/rtti.hpp>
////////////////////////////////////////////////////////////////////////////////
#include <map>
#include <string>


eco_namespace(eco);
eco_namespace(rtti);
////////////////////////////////////////////////////////////////////////////////
struct type_registry_impl
{
	std::map<std::string, const eco::rtti::type*> map;
};


type_registry_impl g_impl;
////////////////////////////////////////////////////////////////////////////////
void type_registry::set(const char* name, const eco::rtti::type* type)
{
    g_impl.map[name] = type;
} 
const eco::rtti::type* type_registry::get(const char* name)
{
    auto it = g_impl.map.find(name);
    return (it != g_impl.map.end()) ? it->second : nullptr;
} 
eco::rtti::object::ptr type_registry::create(const char* name)
{
    const eco::rtti::type* type = get_type(name);
    return (type != nullptr) ? type->create() : eco::rtti::object::ptr();
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rtti);
eco_namespace_end(eco);