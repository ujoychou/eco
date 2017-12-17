#ifndef ECO_RX_APP_H
#define ECO_RX_APP_H
/*******************************************************************************
@ name

@ function

@ exception

@ note
1.ARX: AutoCAD Runtime Extension

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Type.h>
#include <eco/App.h>
#include <eco/DllObject.h>


namespace eco {;


////////////////////////////////////////////////////////////////////////////////
// app message send to erx dll.
enum RxMessageId
{
	rx_msg_init_app		= 0x0001,
	rx_msg_init_cmd		= 0x0002,
	rx_msg_exit_app		= 0x0003,
};


typedef eco::Result (*RxEntryPoint)(IN eco::RxMessageId msg, IN void* ap);
////////////////////////////////////////////////////////////////////////////////
class ECO_API RxDll : public DllObject
{
public:
	inline RxDll(
		IN const char* dll_name,
		IN const char* dll_path)
		: m_rx_msg(0)
	{
		set_dll(dll_name, dll_path);
		m_entry_point = cast_function<RxEntryPoint>("erx_entry_point");
	}

	// notify erx when app init.
	inline eco::Result on_init_app(IN eco::App* ap)
	{
		m_rx_msg = rx_msg_init_app;
		return m_entry_point(RxMessageId(m_rx_msg), ap);
	}

	// notify erx init command.
	inline eco::Result on_init_cmd()
	{
		m_rx_msg = rx_msg_init_cmd;
		return m_entry_point(RxMessageId(m_rx_msg), nullptr);
	}

	// notify erx when app exit.
	inline eco::Result on_exit_app()
	{
		m_rx_msg = rx_msg_exit_app;
		return m_entry_point(RxMessageId(m_rx_msg), nullptr);
	}

	inline int get_cur_message() const
	{
		return m_rx_msg;
	}

private:
	int m_rx_msg;
	RxEntryPoint m_entry_point;
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API RxApp
{
protected:
	// notify erx when app init.
	virtual eco::Result on_init_app()
	{
		return eco::ok;
	}

	// notify erx init command.
	virtual eco::Result on_init_cmd()
	{
		return eco::ok;
	}

	// notify erx when app exit.
	virtual eco::Result on_exit_app()
	{
		return eco::ok;
	}

public:
	inline RxApp()
	{
		m_app = nullptr;
	}

	// get app who load erx.
	inline eco::App& app()
	{
		return *m_app;
	}
	inline const eco::App& get_app() const
	{
		return *m_app;
	}

	// get app config.
	inline const eco::Config& get_sys_config() const
	{
		return get_app().get_sys_config();
	}
	const eco::Config& get_config() const
	{
		return get_app().get_config();
	}

	// erx dll entry point.
	inline eco::Result entry_point(
		IN const eco::RxMessageId msg,
		IN void* ap)
	{
		switch (msg)
		{
		case eco::rx_msg_init_app:
			m_app = static_cast<eco::App*>(ap);
			return on_init_app();
		case eco::rx_msg_init_cmd:
			return on_init_cmd();
		case eco::rx_msg_exit_app:
			return on_exit_app();
		}
		return eco::ok;
	}

private:
	eco::App* m_app;
};


////////////////////////////////////////////////////////////////////////////////
#define ECO_RX(rx_class, get_erx) \
inline rx_class& get_erx() \
{\
	static rx_class s_rx_app;\
	return s_rx_app;\
}\
extern "C" inline eco::Result ECO_EXPORT_API \
	erx_entry_point(IN eco::RxMessageId msg, IN void* ap)\
{\
	return get_erx().entry_point(msg, ap);\
}


////////////////////////////////////////////////////////////////////////////////
}
#endif