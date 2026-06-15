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
static type_registry_impl& get_impl()
{
    static type_registry_impl impl;
    return impl;
}


////////////////////////////////////////////////////////////////////////////////
void type_registry::set_type(const char* name, const eco::rtti::type* type)
{
    get_impl().map[name] = type;
} 
const eco::rtti::type* type_registry::get_type(const char* name)
{
    auto& impl = get_impl();
    auto it = impl.map.find(name);
    return (it != impl.map.end()) ? it->second : nullptr;
} 
eco::rtti::object::ptr type_registry::create(const char* name)
{
    const eco::rtti::type* type = get_type(name);
    return (type != nullptr) ? type->create() : eco::rtti::object::ptr();
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rtti);
eco_namespace_end(eco);