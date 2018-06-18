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
public:
	/*@ open csv file.*/
	virtual void open(IN const char* csv_file, IN const char* mode) = 0;

	/*@ write data to csv file.*/
	virtual void write(IN const char* data, IN const int size) = 0;

	/*@ read data to csv file.*/
	virtual void read(OUT std::string& data) {}

public:
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

	/*@ append object to csv source.*/
	template<typename meta_t, typename object_t>
	inline void append_head(
		IN const object_t& obj,
		IN const ObjectMapping& mapping)
	{
		std::string data;
		get_head(data, mapping);
		write(data.c_str(), data.size());
	}

	/*@ read object set from csv source.*/
	template<typename meta_t, typename object_set_t>
	inline void read_all(
		IN const std::string& x,
		IN const ObjectMapping& mapping,
		OUT object_set_t& obj_set)
	{
		std::vector<const eco::PropertyMapping*> prop_set;
		uint32_t index = 0;
		uint32_t find = eco::find_first(&x[index], '\n');
		if (find != -1)
		{
			std::string head = x.substr(index, find);
			uint32_t pos = eco::find_first(&head[index], ',');
			while (pos != -1) {
				const eco::PropertyMapping* pm = mapping.find_property(
					head.substr(index, pos).c_str());
				index += (pos + 1);
				pos = eco::find_first(&head[index], ',');
				if (pm == nullptr)
					continue;

				prop_set.push_back(pm);
			}

			pos = head.size() - index - 1;
			const eco::PropertyMapping* pm = mapping.find_property(
				head.substr(index, pos).c_str());
			if (pm != nullptr)
				prop_set.push_back(pm);

			index += (pos + 1);
		}

		do
		{
			index += 1;
			find = eco::find_first(&x[index], '\n');
			if (find == -1)
				break;

			meta_t meta;
			auto obj = meta.create();
			meta.attach(obj);

			uint32_t pos = eco::find_first(&x[index], ',');
			for (size_t i = 0; pos != -1 && i < prop_set.size(); ++i)
			{
				eco::Bytes val(&x[index], pos);
				index += (pos + 1);

				if (i == prop_set.size() - 2)
					pos = eco::find_first(&x[index], '\r');
				else
					pos = eco::find_first(&x[index], ',');

				meta.set_value(prop_set[i]->get_field_index(), val, "");
			}
			obj_set.push_back(obj);
		} while (1);
	}

public:
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
class ECO_API CsvSourceFile : public CsvSource
{
	ECO_SHARED_API(CsvSourceFile);
public:
	/*@ open csv file.*/
	virtual void open(IN const char* csv_file, IN const char* mode) override;

	/*@ write data to csv file.*/
	virtual void write(IN const char* data, IN const int size) override;

	/*@ read data to csv file.*/
	virtual void read(OUT std::string& data) override;
};


////////////////////////////////////////////////////////////////////////////////
}//ns
#endif
