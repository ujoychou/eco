#ifndef ECO_LOC_LANGUAGE_H
#define ECO_LOC_LANGUAGE_H
/*******************************************************************************
@ name
log server.

@ function

@ exception

@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2020-07-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2020 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>
#include <eco/Config.h>
#include <eco/eco/Proto.h>
#include <eco/filesystem/Path.h>
#include <eco/thread/Thread.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(loc);
////////////////////////////////////////////////////////////////////////////////
class Language : public eco::Object<Language>
{
public:
	inline Language()
	{}

	inline static void get_path(
		std::string& key, const char* m, const std::string& v)
	{
		key = m;
		if (!eco::empty(m)) key += '/';
		key += v;
	}
	inline static void get_path(std::string& key, const char* m, int id)
	{
		key = m;
		if (!eco::empty(m)) key += '/';
		key += eco::Integer<int>(id).c_str();
	}

	// init word and error by locale data.
	inline void add_language(
		OUT eco::proto::Language& obj,
		IN  const eco::proto::Language& new_t,
		IN  const char* mdl)
	{
		std::string key;
		for (int i = 0; i < new_t.word_size(); ++i)
		{
			const eco::proto::Word& wrd = new_t.word(i);
			get_path(key, mdl, wrd.path());
			eco::proto::Word* w = obj.add_word();
			w->set_path(key);
			w->set_value(wrd.value());
			m_word_map[key] = wrd.value();
		}
		for (int i = 0; i < new_t.error_size(); ++i)
		{
			const eco::proto::Error& err = new_t.error(i);
			if (!err.path().empty())
				get_path(key, mdl, err.path());
			else if (err.id() != 0)
				get_path(key, mdl, err.id());
			else
				continue;
			eco::proto::Error* e = obj.add_error();
			e->set_id(err.id());
			e->set_path(key);
			e->set_value(err.value());
			m_error_map[key] = err.value();
		}
	}

	// init error object file.
	inline void add_error_file(
		eco::proto::Language& lang, const char* file, std::string mdl)
	{
		// read error object.
		eco::Config cfg;
		cfg.init(file);
		eco::ContextNode root = cfg.find_node();
		if (!root.null())
		{
			if (!mdl.empty()) mdl += '/';
			add_error_node(lang, root, mdl);
		}
	}

	// init error object file.
	inline void add_word_file(
		eco::proto::Language& lang, const char* file, std::string mdl)
	{
		// read error object.
		eco::Config cfg;
		cfg.init(file);
		eco::ContextNode root = cfg.find_node();
		if (!root.null())
		{
			if (!mdl.empty()) mdl += '/';
			add_word_node(lang, root, mdl);
		}
	}

	// make error node.
	inline void make_error(eco::proto::Error& e,
		const char* n, const char* v, const std::string& path)
	{
		e.set_value(v);
		std::string name = get_node_name(n);
		if (path.empty() && eco::is_number(name.c_str()))
		{
			e.set_id(eco::cast<int>(name));
			m_ecode_map[e.id()] = e.value();
		}
		else
		{
			e.set_path(path.empty() ? name : path + name);
			m_error_map[e.path()] = e.value();
		}
	}

	// init error object.
	inline void add_error_node(
		eco::proto::Language& lang, 
		eco::ContextNode& node,
		const std::string& path)
	{
		// property set is error object.
		const eco::Context& errs = node.property_set();
		for (auto ite = errs.begin(); ite != errs.end(); ++ite)
		{
			const eco::Parameter& param = *ite;
			eco::proto::Error* e = lang.add_error();
			make_error(*e, param.name(), param.value(), path);
		}

		// recursively context node to get leaf error.
		eco::ContextNodeSet node_set = node.children();
		if (!node_set.null())
		{
			for (auto itn = node_set.begin(); itn != node_set.end(); ++itn)
			{
				eco::ContextNode& node = *itn;
				if (!eco::empty(node.value()))
				{
					eco::proto::Error* e = lang.add_error();
					make_error(*e, node.name(), node.value(), path);
				}

				std::string path_cur = path;
				path_cur += get_node_name(node.name());
				path_cur += '/';
				add_error_node(lang, node, path_cur);
			}
		}
	}

public:
	// make error node.
	inline void make_word(eco::proto::Word& w, 
		const char* n, const char* v, const std::string& path)
	{
		std::string name = get_node_name(n);
		w.set_value(v);
		w.set_path(path.empty() ? name : path + name);
		m_word_map[w.path()] = w.value();
	}

	// init error object.
	inline void add_word_node(
		eco::proto::Language& lang, 
		eco::ContextNode& node, 
		const std::string& path)
	{
		// property set is error object.
		const eco::Context& errs = node.property_set();
		for (auto ite = errs.begin(); ite != errs.end(); ++ite)
		{
			const eco::Parameter& param = *ite;
			eco::proto::Word* w = lang.add_word();
			make_word(*w, param.name(), param.value(), path);
		}

		// recursively context node to get leaf error.
		eco::ContextNodeSet node_set = node.children();
		if (!node_set.null())
		{
			for (auto itn = node_set.begin(); itn != node_set.end(); ++itn)
			{
				eco::ContextNode& node = *itn;
				if (!eco::empty(node.value()))
				{
					eco::proto::Word* w = lang.add_word();
					make_word(*w, node.name(), node.value(), path);
				}

				std::string path_cur = path;
				path_cur += get_node_name(node.name());
				path_cur += '/';
				add_word_node(lang, node, path_cur);
			}
		}
	}

public:
	// get error message by error id.
	inline const char* get_error(IN int eid, const char* para)
	{
		auto it = m_ecode_map.find(eid);
		const char* msg = (it != m_ecode_map.end())	? it->second.c_str() : "";
		if (eco::empty(para)) return msg;

		eco::FormatX& fmt = this_thread::format(msg);
		fmt.arg(';', para);
		return fmt.c_str();
	}

	// get error message by string path.
	inline const char* get_error(const std::string& path, const char* para)
	{
		auto it = m_error_map.find(path);
		const char* msg = (it != m_error_map.end())	? it->second.c_str() : "";
		if (eco::empty(para)) return msg;

		auto& fmt = this_thread::format(msg);
		fmt.arg(';', para);
		return fmt.c_str();
	}

	// get word view.
	inline const char* get_word(const std::string& path)
	{
		auto it = m_word_map.find(path);
		return (it != m_word_map.end()) ? it->second.c_str() : "";
	}

private:
	inline std::string get_node_name(const char* name)
	{
		return (name[0] != 0 && name[1] != 0)
			&& (name[0] == 'n' || name[0] == 'N')
			&& eco::is_number(name[1]) ? &name[1] : name;
	}

	inline bool is_seperator(char c)
	{
		return c == '.' || c == '-' || c == '_';
	}

	std::unordered_map<int, std::string> m_ecode_map;
	std::unordered_map<std::string, std::string> m_word_map;
	std::unordered_map<std::string, std::string> m_error_map;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(loc);
ECO_NS_END(eco);
#endif