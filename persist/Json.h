#ifndef ECO_JSON_H
#define ECO_JSON_H
/*******************************************************************************
@ name

@ function

@ remark
1.generate json string format:
var json = {
	p1: "v1",
	p2: "v2",
	p3: "v3",
	array: [{
		p1: "v1",
		p1: "v2",
	},{
		p1: "v1",
		p1: "v2",
	}]
}

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-06-10.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/persist/ObjectMapping.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>


namespace eco {;

////////////////////////////////////////////////////////////////////////////////
class Json
{
public:
	template<typename meta_t, typename string_t, typename object_t>
	inline static void format(
		OUT string_t& json,
		IN  const object_t& obj,
		IN  const ObjectMapping& obj_mapping)
	{
		meta_t meta;
		meta.attach(obj);

		// empty object.
		auto map = obj_mapping.get_map();
		if (map.empty())
		{
			json = "{}";
			return;
		}

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		writer.StartObject();
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			writer.Key(it->get_field());
			writer.String(
				meta.get_value(it->get_property(), "eco_json").c_str());
		}
		writer.EndObject();
		json = buffer.GetString();
	}

	template<typename meta_t, typename string_t, typename object_set_t>
	inline static void format_array(
		OUT string_t& json_array,
		IN  const object_set_t& obj_set,
		IN  const ObjectMapping& obj_mapping)
	{
		// empty array.
		if (obj_set.begin() == obj_set.end() || obj_mapping.get_map().empty())
		{
			json_array = "[]";
			return;
		}

		meta_t meta;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		writer.StartArray();
		for (auto it = obj_set.begin(); it != obj_set.end(); ++it)
		{
			meta.attach(*it);
			auto& map = obj_mapping.get_map();
			writer.StartObject();
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				writer.Key(it->get_field());
				writer.String(
					meta.get_value(it->get_property(), "eco_json").c_str());
			}
			writer.EndObject();
		}
		writer.EndArray();
		json_array = buffer.GetString();
	}

public:
	template<typename meta_t, typename object_t>
	inline static void parse(
		OUT object_t& obj,
		IN  const std::string& json,
		IN  const ObjectMapping& obj_mapping)
	{
		meta_t meta;
		meta.attach(obj);

		rapidjson::Document doc;
		doc.Parse(json.c_str());
		auto it = obj_mapping.get_map().begin();
		for (; it != obj_mapping.get_map().end(); ++it)
		{
			meta.set_value(it->get_property(), doc[it->get_field()]);
		}
	}

	template<typename meta_t, typename object_set_t>
	inline static void parse_array(
		OUT object_set_t& obj_set,
		IN  const std::string& json,
		IN  const ObjectMapping& obj_mapping,
		IN  const char* child_prop = nullptr)
	{
		meta_t meta;
		rapidjson::Document doc;
		doc.Parse(json.c_str());
		rapidjson::Value& jsv = (child_prop == nullptr) ? doc : doc[child_prop];
		for (auto vit = jsv.Begin(); vit != jsv.End(); ++vit)
		{
			object_set_t::value_type obj(meta.create());
			meta.attach(obj);

			auto it = obj_mapping.get_map().begin();
			for (; it != obj_mapping.get_map().end(); ++it)
			{
				meta.set_value(it->get_property(), (*vit)[it->get_field()]);
			}
			obj_set.push_back(obj);
		}
	}
};



////////////////////////////////////////////////////////////////////////////////
}//ns
#endif
