#include "Pch.h"
#include <eco/loc/Locale.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/App.h>
#include "Language.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(loc);
////////////////////////////////////////////////////////////////////////////////
class Locale::Impl
{
	ECO_IMPL_INIT(Locale);
public:
	eco::proto::Locale m_locale;
	std::unordered_map<std::string, Language::ptr> m_lang_map;

	// set default language.
	inline void set_default(const char* lang, bool force)
	{
		if (m_locale.default_().empty() || force)
		{
			m_locale.set_default_(lang);
		}
	}
	inline void set_language_info(const char* l, const char* p, const char* v)
	{
		set_default(l, false);
		eco::proto::Language& data = get_lang_data(l);
		data.set_path(p);
		data.set_version(v);
	}

	// add error xml
	inline void add_error_file(const char* lang, const char* f, const char* m)
	{
		get_lang(lang).add_error_file(get_lang_data(lang), f, m);
	}

	// add word xml
	inline void add_word_file(const char* lang, const char* f, const char* m)
	{
		get_lang(lang).add_word_file(get_lang_data(lang), f, m);
	}

	// add module's locale that app depends.
	inline void add_locale(const proto::Locale& loc, const char* m)
	{
		for (int i = 0; i < loc.lang_size(); ++i)
		{
			const eco::proto::Language& obj = loc.lang(i);
			const char* n = obj.name().c_str();
			get_lang(n).add_language(get_lang_data(n), obj, m);
		}
	}

public:
	// get error message by error id.
	inline const char* get_error(int eid, const char* lang)
	{
		return get_lang(lang).get_error(eid, "");
	}
	inline const char* get_error(
		int eid, const char* m, const char* para, const char* lang)
	{
		if (!eco::empty(m))
		{
			std::string key;
			Language::get_path(key, m, eid);
			return get_lang(lang).get_error(key, para);
		}
		return get_lang(lang).get_error(eid, para);
	}

	// get error message by string path.
	inline const char* get_error(
		const std::string& path, const char* para, const char* lang)
	{
		auto* obj = find_lang(lang);
		return obj ? obj->get_error(path, para) : "";
	}

	// get word view.
	inline const char* get_word(
		const std::string& path, const char* mdl, const char* lang)
	{
		auto* obj = find_lang(lang);
		if (obj == nullptr) return "";

		if (!eco::empty(mdl))
		{
			std::string temp(mdl);
			temp += '/';
			temp += path;
			return obj->get_word(temp);
		}
		return obj->get_word(path);
	}

	inline const char* default_name(const char* name_) const
	{
		return !eco::empty(name_) ? name_ : m_locale.default_().c_str();
	}

	// get language object.
	inline Language& get_lang(const char* name_)
	{
		name_ = default_name(name_);
		Language::ptr& lang = m_lang_map[name_];
		if (lang == nullptr) lang.reset(new Language());
		return *lang;
	}

	// get language object.
	inline Language* find_lang(const char* name_) const
	{
		name_ = default_name(name_);
		auto it = m_lang_map.find(name_);
		return it != m_lang_map.end() ? it->second.get() : nullptr;
	}

	// get word view.
	inline const char* get_variable(
		const char* var, const char* mdl, const char* lang)
	{
		auto* obj = find_lang(lang);
		if (!obj) return "";

		std::string key;
		const char* pos_last = var;
		eco::String& buf = eco::this_thread::logbuf().stream();
		for (buf.clear();;)
		{
			uint32_t pos_s = eco::find_first(pos_last, '{');
			if (pos_s == -1) break;
			uint32_t pos_e = eco::find_first(pos_last + (++pos_s), '}');
			if (pos_e == -1) break;

			key = mdl; key += '/';
			key.append(pos_last + pos_s, pos_e);
			buf.append(pos_last, pos_s - 1);
			buf.append(obj->get_word(key));
			pos_last = pos_last + pos_s + (++pos_e);
		}
		buf.append(pos_last);
		return buf.c_str();
	}

public:
	// get language proto.
	inline eco::proto::Language* find_lang_data(const char* name_)
	{
		name_ = default_name(name_);
		for (int i = 0; i < m_locale.lang_size(); ++i)
		{
			eco::proto::Language& lang = *m_locale.mutable_lang(i);
			if (lang.name() == name_) return &lang;
		}
		return nullptr;
	}

	// get language proto.
	inline const eco::proto::Language* find_lang_data(const char* name_) const
	{
		Locale::Impl* impl = (Locale::Impl*)this;
		return impl->find_lang_data(name_);
	}

	// get language proto.
	inline eco::proto::Language& get_lang_data(const char* name_)
	{
		eco::proto::Language* data = find_lang_data(name_);
		if (data == nullptr)
		{
			data = m_locale.add_lang();
			data->set_name(name_);
		}
		return *data;
	}

	bool find(eco::StringAny& v, const char* key, const char* name_) const
	{
		std::string loc_key = "locale";
		name_ = default_name(name_);
		loc_key += '/';
		loc_key += name_;
		loc_key += '/';
		loc_key += key;
		return eco::App::get()->config().find(v, loc_key.c_str());
	}
};


ECO_OBJECT_IMPL(Locale);
////////////////////////////////////////////////////////////////////////////////
eco::loc::Locale& app_locale()
{
	return eco::App::get()->locale();
}
////////////////////////////////////////////////////////////////////////////////
void Locale::add_locale(const eco::proto::Locale& loc, const char* mdl)
{
	impl().add_locale(loc, mdl);
}
void Locale::set_default_(const char* lang)
{
	impl().set_default(lang, true);
}
void Locale::set_language_info(const char* lang, const char* path, const char* ver)
{
	impl().set_language_info(lang, path, ver);
}
void Locale::add_error_file(const char* lang, const char* f, const char* m)
{
	impl().add_error_file(lang, f, m);
}
void Locale::add_word_file(const char* lang, const char* file, const char* m)
{
	impl().add_word_file(lang, file, m);
}
const char* Locale::default_() const
{
	return impl().m_locale.default_().c_str();
}


////////////////////////////////////////////////////////////////////////////////
const char* Locale::get_error(int eid, const char* lang)
{
	return impl().get_error(eid, lang);
}
eco::FormatX& Locale::get_error_format(int eid, const char* lang)
{
	return this_thread::format()(impl().get_error(eid, lang));
}
eco::FormatX& Locale::get_error_format(const char* path, const char* lang)
{
	return this_thread::format()(impl().get_error(path, "", lang));
}
const char* Locale::get_word(
	const char* path, const char* mdl, const char* lang)
{
	return impl().get_word(path, mdl, lang);
}
eco::FormatX& Locale::get_word_format(
	const char* path, const char* mdl, const char* lang)
{
	return this_thread::format()(impl().get_word(path, mdl, lang));
}
const char* Locale::get_variable(const char* p, const char* m, const char* l)
{
	return impl().get_variable(p, m, l);
}
eco::FormatX& Locale::get_variable_format(
	const char* path, const char* mdl, const char* lang)
{
	return this_thread::format()(impl().get_variable(path, mdl, lang));
}


////////////////////////////////////////////////////////////////////////////////
const char* Locale::parse_error(
	const char* path, const char* para, const char* mdl, const char* lang)
{
	if (!eco::empty(mdl))
	{
		std::string key = mdl; key += '/'; key += path;
		return this_thread::format()(impl().get_error(key, para, lang));
	}
	return impl().get_error(path, para, lang);
}
const char* Locale::parse_error(
	int eid, const char* para, const char* mdl, const char* lang)
{
	return impl().get_error(eid, mdl, para, lang);
}


////////////////////////////////////////////////////////////////////////////////
bool Locale::find(eco::StringAny& v, const char* key, const char* lang) const
{
	return impl().find(v, key, lang);
}
eco::StringAny Locale::get(const char* key, const char* lang) const
{
	eco::StringAny v;
	impl().find(v, key, lang);
	return v;
}
const eco::proto::Locale& Locale::data() const
{
	return impl().m_locale;
}
const eco::proto::Language* Locale::language(const char* lang) const
{
	return impl().find_lang_data(lang);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(loc);
ECO_NS_END(eco);