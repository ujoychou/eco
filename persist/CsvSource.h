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
#include <eco/rx/RxApi.h>
#include <eco/persist/Recordset.h>
#include <eco/persist/ObjectMapping.h>


ECO_NS_BEGIN(eco);
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
	// get csv head.
	inline void get_head(
		OUT std::string& data,
		IN  const ObjectMapping& mapping)
	{
		if (mapping.map().empty()) return;

		// csv head.
		auto it = mapping.map().begin();
		for (; it != mapping.map().end(); ++it)
		{
			data += it->field();
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
		if (mapping.map().empty()) return;

		// csv data.
		meta_t meta;
		meta.attach(obj);
		auto it = mapping.map().begin();
		for (; it != mapping.map().end(); ++it)
		{
			data += meta.get_value(it->property(), "eco_csv");
			data += ',';					// tab
		}
		data.at(data.size() - 1) = '\n';	// enter
	}

public:
	/*@ save object set to csv source.*/
	template<typename meta_t, typename object_set_t>
	inline void save(
		IN const object_set_t& obj_set,
		IN const ObjectMapping& mapping)
	{
		std::string data;
		get_head(data, mapping);

		typename object_set_t::const_iterator it = obj_set.begin();
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
		OUT object_set_t& obj_set,
		IN const std::string& data,
		IN const ObjectMapping& mapping)
	{
		std::vector<const eco::PropertyMapping*> prop_set;
		// parse header field.
		auto row_end = parse_property(prop_set, data.c_str(), mapping);
		if (row_end == nullptr) return;

		// parse row data.
		const char* row_start = row_end + 1;
		for (uint32_t row = eco::find_first(row_start, '\n'); row != -1; )
		{
			row_end = row_start + row;
			parse_object<meta_t>(obj_set, row_start, row_end, prop_set);

			// parse next object by row.
			row_start = row_end + 1;
			row = eco::find_first(row_start, '\n');
		}
		// parse row 
		row_end = data.c_str() + data.size();
		parse_object<meta_t>(obj_set, row_start, row_end, prop_set);
	}

	/*@ read object set from csv source.*/
	template<typename meta_t, typename object_set_t>
	inline void read_all_by_file(
		OUT object_set_t& obj_set,
		IN const std::string& file,
		IN const ObjectMapping& mapping)
	{}

private:
	inline const char* parse_property(
		OUT std::vector<const eco::PropertyMapping*>& prop_set,
		IN  const char* row_start,
		IN  const ObjectMapping& mapping)
	{
		// parse header field.
		const char* fdata = row_start;
		uint32_t row = eco::find_first(fdata, '\n');
		if (row == -1) return nullptr;

		int fi = 0;
		const char* row_end = fdata + row;
		uint32_t pos = eco::find_first(fdata, ',');
		for (; pos != -1 && fdata + pos < row_end; ++fi)
		{
			PropertyMapping* pm = mapping.find_field_n(fdata, pos);
			if (pm != nullptr)
			{
				pm->field_index(fi);
				prop_set.push_back(pm);
			}
			fdata = &fdata[pos + 1];
			pos = eco::find_first(fdata, ',');
		}
		// parse header field: last.
		auto pm = mapping.find_field_n(fdata, row_end - fdata);
		if (pm != nullptr)
		{
			pm->field_index(fi);
			prop_set.push_back(pm);
		}
		return row_end;
	}

	template<typename meta_t, typename object_set_t>
	inline void parse_object(
		OUT object_set_t& obj_set,
		IN  const char* row_start,
		IN  const char* row_end,
		IN  const std::vector<const eco::PropertyMapping*>& prop_set)
	{
		// parse object property.
		const char* fdata = row_start;
		uint32_t pos = eco::find_first(fdata, ',');
		if (pos == -1) return;

		// create object.
		typename object_set_t::value_type obj;
		meta_t meta;
		meta.make_attach(obj);
		uint32_t fi = 0;
		uint32_t pi = 0;
		for (; pos != -1 && pi < prop_set.size() && fdata + pos < row_end; ++fi)
		{
			eco::Bytes val(fdata, pos);
			if (prop_set[pi]->field_index() == fi)
			{
				meta.set_value(*prop_set[pi++], val, "eco_csv");
			}
			fdata = &fdata[pos + 1];
			pos = eco::find_first(fdata, ',');
		}
		if (pi < prop_set.size())
		{
			eco::Bytes val(fdata, row_end - fdata);
			if (prop_set[pi]->field_index() == fi)
			{
				meta.set_value(*prop_set[pi++], val, "eco_csv");
			}
		}
		obj_set.push_back(obj);
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
	virtual void write(IN const char* data, IN int size) override;

	/*@ read data to csv file.*/
	virtual void read(OUT std::string& data) override;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
