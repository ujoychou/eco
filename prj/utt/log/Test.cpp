#include "PrecHeader.h"
#include "Test.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/Error.h>
#include "App.h"



namespace eco{;
namespace log{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
void MtCommand::execute(IN const eco::cmd::Context& context)
{
	if (context.size() < 1)
	{
		std::cout << "mt command parameter is invalid.";
		return ;
	}

	int times = context.at(0);
	m_thread_pool.run(
		std::bind(&MtCommand::log_level, this, times), 1, "TL1");
	m_thread_pool.run(
		std::bind(&MtCommand::log_level, this, times), 1, "TL2");
	m_thread_pool.run(
		std::bind(&MtCommand::log_level, this, times), 1, "TL3");
	m_thread_pool.run(
		std::bind(&MtCommand::log_domain, this, times, "Good"), 1, "TD1");
	m_thread_pool.run(
		std::bind(&MtCommand::log_domain, this, times, "Wood"), 1, "TD2");
	m_thread_pool.run(
		std::bind(&MtCommand::log_domain, this, times, "Mood"), 1, "TD3");
}


////////////////////////////////////////////////////////////////////////////////
void MtCommand::log_level(IN size_t loop_times)
{
	eco::log::get_core().set_severity_level(eco::log::trace);
	for (size_t i=0; i<loop_times; ++i)
	{
		EcoTrace << "this is a long long message, very long long long message, "
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "this is a long long message, very long long long messagexxxxx, "
			<< "this is a long long message, very long long long messagexxxxx, "
			<< " " << 100899238 << " " << 987722344.89873 << " " << true;

		EcoDebug << "this is a long long message, very long long long message, "
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "this is a long long message, very long long long messagexxxxx, "
			<< "this is a long long message, very long long long messagexxxxx, "
			<< " " << 100899238 << " " << 987722344.89873 << " " << true;

		EcoInfo << "this is a long long message, very long long long message, "
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "this is a long long message, very long long long messagexxxxx, "
			<< "this is a long long message, very long long long messagexxxxx, "
			<< " " << 100899238 << " " << 987722344.89873 << " " << true;

		EcoWarn << "this is a long long message, very long long long message, "
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "this is a long long message, very long long long messagexxxxx, "
			<< "this is a long long message, very long long long messagexxxxx, "
			<< " " << 100899238 << " " << 987722344.89873 << " " << true;

		EcoError << "this is a long long message, very long long long message, "
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "this is a long long message, very long long long messagexxxxx, "
			<< "this is a long long message, very long long long messagexxxxx, "
			<< " " << 100899238 << " " << 987722344.89873 << " " << true;

		EcoFatal << "this is a long long message, very long long long message, "
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "this is a long long message, very long long long messagexxxxx, "
			<< "this is a long long message, very long long long messagexxxxx, "
			<< " " << 100899238 << " " << 987722344.89873 << " " << true;
	}
	EcoCout << "thread: " << eco::this_thread::id_string()
		<< eco::this_thread::name();
}


////////////////////////////////////////////////////////////////////////////////
void MtCommand::log_domain(IN size_t loop_times, IN const char* domain)
{
	eco::log::get_core().set_severity_level(eco::log::info);
	for (size_t i=0; i<loop_times; ++i)
	{
		EcoLog(warn, domain) << "it is a long long message, very long long long, "
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
			<< "this is a long long message, very long long long messagexxxxx, "
			<< "this is a long long message, very long long long messagexxxxx, "
			<< " " << 100899238 << " " << 987722344.89873 << " " << true;
	}
	EcoCout << "thread: " << eco::this_thread::id_string()
		<< eco::this_thread::name();
}


////////////////////////////////////////////////////////////////////////////////
void FuncCommand::execute(IN const eco::cmd::Context& context)
{
	int x = context.size();
	if (context.size() < 2)
	{
		EcoCout << "mt command parameter is invalid.";
		return ;
	}

	const char* level  = context.at(0);
	const char* domain = context.at(1);
	log_test(level, domain);
}


////////////////////////////////////////////////////////////////////////////////
void FuncCommand::log_test(IN const char* level, IN const char* domain)
{
	eco::log::get_core().set_severity_level(level);
	EcoTrace << "this is a long long message, very long long long message, "
		<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
		<< " " << 100899238 << " " << 987722344.89873 << " " << true;
	EcoDebug << "this is a long long message, very long long long message, "
		<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
		<< " " << 100899238 << " " << 987722344.89873 << " " << true;
	EcoInfo << "this is a long long message, very long long long message, "
		<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
		<< " " << 100899238 << " " << 987722344.89873 << " " << true;
	EcoWarn << "this is a long long message, very long long long message, "
		<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
		<< " " << 100899238 << " " << 987722344.89873 << " " << true;
	EcoError << "this is a long long message, very long long long message, "
		<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
		<< " " << 100899238 << " " << 987722344.89873 << " " << true;
	EcoFatal << "this is a long long message, very long long long message, "
		<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
		<< " " << 100899238 << " " << 987722344.89873 << " " << true;

	EcoLog(info, domain)
		<< "this is a long long message, very long long long message, "
		<< "很长的中文与英文的字符串，非常非常非常非常非常非常非常长xxxxx，"
		<< " " << 100899238 << " " << 987722344.89873 << " " << true;
}


////////////////////////////////////////////////////////////////////////////////
}}}