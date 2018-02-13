#include "PrecHeader.h"
#include <eco/Config.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/thread/Map.h>
#include <eco/xml/Reader.h>



namespace eco{;
//##############################################################################
//##############################################################################
class Config::Impl
{
public:
	void init(Config&) {}

	// config nodes.
	ContextNode m_root;

	// index of config nodes.
	eco::HashMap<std::string, StringAny> m_data;

public:
	inline void get_property_set_raw(
		OUT eco::Context& result,
		IN  const char* parent_key,
		IN  const ContextNodeSet& node_set) const
	{
		int node_end = eco::find_first(parent_key, '/');
		for (auto it = node_set.begin(); it != node_set.end(); ++it)
		{
			// find the key node.
			if (node_end == -1 && strcmp(it->get_name(), parent_key) == 0)
			{
				result = it->get_property_set();
				return;
			}

			// find the next level node.
			if (node_end != -1 &&
				strncmp(it->get_name(), parent_key, node_end) == 0)
			{
				if (it->has_children())
				{
					get_property_set_raw(result,
						&parent_key[node_end + 1], it->get_children());
				}
				return;
			}
		}
	}

	inline void get_children_raw(
		OUT eco::ContextNodeSet& result,
		IN  const char* parent_key,
		IN  const ContextNodeSet& node_set) const
	{
		int node_end = eco::find_first(parent_key, '/');
		for (auto it = node_set.begin(); it != node_set.end(); ++it)
		{
			// find the key node.
			if (node_end == -1 && strcmp(it->get_name(), parent_key) == 0)
			{
				if (it->has_children())
				{
					result = it->get_children();
				}
				return;
			}

			// find the next level node.
			if (node_end != -1 && 
				strncmp(it->get_name(), parent_key, node_end) == 0)
			{
				if (it->has_children())
				{
					get_children_raw(result, 
						&parent_key[node_end + 1], it->get_children());
				}
				return;
			}
		}
	}// end

	void init(IN const std::string& file)
	{
		try
		{
			xml::Reader reader;
			reader.read(m_root, file.c_str());

			// setup index for config key.
			m_root.set_name("");
			setup_index(m_root, "");
		}
		catch (const std::exception& e)
		{
			EcoThrow << "init config fail, " << e.what();
		}
	}

	inline void get_property_set(
		OUT eco::Context& result,
		IN  const char* node_key) const
	{
		eco::Mutex::ScopeLock lock(m_data.mutex());
		if (m_root.has_children())
		{
			get_property_set_raw(result, node_key, m_root.get_children());
		}
	}

	inline void get_children(
		OUT eco::ContextNodeSet& result,
		IN  const char* parent_key) const
	{
		eco::Mutex::ScopeLock lock(m_data.mutex());
		if (m_root.has_children())
		{
			get_children_raw(result, parent_key, m_root.get_children());
		}
	}

	inline void setup_index(
		IN const eco::ContextNode& node,
		IN const std::string& parent_name)
	{
		// node name.
		std::string node_name(parent_name);
		eco::xml::sub_path(node_name, node.get_name());
		if (!node_name.empty())
		{
			m_data.set(node_name, node.get_value());
		}
	
		// node attribute.
		auto att = node.get_property_set().begin();
		for (; att != node.get_property_set().end(); ++att)
		{
			std::string attr_name(node_name);
			eco::xml::sub_path(attr_name, att->get_key());
			m_data.set(attr_name, att->get_value());
		}

		// node children
		if (node.has_children())
		{
			auto it = node.get_children().begin();
			for (; it != node.get_children().end(); ++it)
			{
				setup_index(*it, node_name);
			}
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_OBJECT_IMPL(Config);
void Config::init(IN const std::string& file)
{
	impl().init(file);
}
bool Config::find(OUT eco::StringAny& v, IN const char* key) const
{
	return impl().m_data.find(v, key);
}
const eco::StringAny Config::at(IN const char* key) const
{
	eco::StringAny v;
	if (!impl().m_data.find(v, key))
	{
		EcoThrow << "config can't find key: " << key;
	}
	return v;
}
void Config::add(IN const char* key, IN const char* value)
{
	StringAny v;
	impl().m_data.set(key, (v = value));
}
void Config::get_children(
	OUT eco::ContextNodeSet& node_set,
	IN const char* parent_key) const
{
	impl().get_children(node_set, parent_key);
}
void Config::get_property_set(
	OUT eco::Context& context_set,
	IN  const char* node_key) const
{
	impl().get_property_set(context_set, node_key);
}


////////////////////////////////////////////////////////////////////////////////
}