#ifndef ECO_CSV_SOURCE_H
#define ECO_CSV_SOURCE_H
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-06-10.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/persist/Recordset.h>
#include <eco/persist/ObjectMapping.h>


namespace eco {;

////////////////////////////////////////////////////////////////////////////////
class ECO_API CsvSource
{
	ECO_SHARED_API(CsvSource);
public:
	/*@ open csv file.*/
	void open(IN const char* csv_file, IN const char* mode);

	/*@ save object set to csv source.*/
	template<typename meta_t, typename object_set_t>
	inline void save(
		IN const object_set_t& obj_set,
		IN const ObjectMapping& mapping)
	{
		std::string data;
		get_head(data, mapping);

		object_set_t::const_iterator it = obj_set.begin();
		for (; it != obj_set.end(); ++it)
		{
			get_data<meta_t>(data, *it, mapping);
		}

		write(data.c_str(), data.size());
	}

	/*@ append object to csv source.*/
	template<typename meta_t, typename object_t>
	inline void append(
		IN const object_t& obj,
		IN const ObjectMapping& mapping)
	{
		std::string data;
		get_data<meta_t>(data, obj, mapping);
		write(data.c_str(), data.size());
	}

	/*@ read object set from csv source.*/
	template<typename meta_t, typename object_set_t>
	inline void read_all(
		IN const object_set_t& obj_set,
		IN const ObjectMapping& mapping)
	{
		Record head;
		Recordset rd_set;
		read(head, rd_set);

		// get property set by mapping.
		std::list<PropertyMapping> prop_set;
		for (int i = 0; i < head.size(); ++i)
		{
			prop_set.push_back(mapping.Find(head.at(i)));
		}
		mapping.decode_some<meta_t>(obj_set, rd_set, prop_set);
	}

	/*@ write data to csv file.*/
	void write(IN const char* data, IN const int size);

	/*@ read data to csv file.*/
	void read(OUT Record& head, IN Recordset& data);

private:
	// get csv head.
	inline void get_head(
		OUT std::string& data,
		IN const ObjectMapping& mapping)
	{
		if (mapping.get_map().empty())
		{
			return;
		}

		// csv head.
		auto it = mapping.get_map().begin();
		for (; it != mapping.get_map().end(); ++it)
		{
			data += it->get_field();
			data += ',';					// tab
		}
		data.at(data.size() - 1) = '\n';	// enter
	}

	// get csv data of object.
	template<typename meta_t, typename object_t>
	inline void get_data(
		OUT std::string& data,
		IN const object_t& obj,
		IN const ObjectMapping& mapping)
	{
		meta_t meta;
		meta.attach(obj);

		if (mapping.get_map().empty())
		{
			return;
		}

		// csv data.
		auto it = mapping.get_map().begin();
		for (; it != mapping.get_map().end(); ++it)
		{
			data += meta.get_value(it->get_property(), "eco_csv");
			data += ',';					// tab
		}
		data.at(data.size() - 1) = '\n';	// enter
	}
};



////////////////////////////////////////////////////////////////////////////////
}//ns
#endif
