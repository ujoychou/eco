#include "PrecHeader.h"
#include "App.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/thread/Thread.h>
#include "Test.h"


namespace eco{;
namespace cmd{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
App::App()
{}


////////////////////////////////////////////////////////////////////////////////
void App::on_cmd()
{
	/***************************************************************************
	1.command group tree:
	--g1
	----c1
	----c2
	----c3
	----c4
	--g2
	----g21
	------c4
	------g211
	--------c3
	--------c4
	--------c5
	----g22
	------c9
	--g3
	----c3
	----c6
	----c7

	2.call case:
	2.1: command group tree.
	2.2: command reference.
	2.3: routine command: cd(.\..);exit;help;
	2.4: derived command.
	2.5: execute func command.

	3.run case:
	3.1: deal cmd with inputting name is empty, same with linux command.
	3.2: 
	/**************************************************************************/

	// g1.c1\2\3\4
	eco::cmd::Group g1 = eco::App::home().add_group().name("g1");
	eco::cmd::Class c1 = g1.add_command().bind<Cmd1>("cmd1 help info in g1");
	auto c2 = g1.add_command().bind<Cmd2>("cmd2 help info in g2");
	auto c3 = g1.add_command().bind("cmd3", "c3",
		"c3 helper info.", Manager::cmd3);
	auto c4 = g1.add_command().bind("cmd4", "c4",
		"c4 helper info.", Manager::cmd4);

	// g2.g21.g211
	auto g2 = eco::App::home().add_group().name("app_group_b").alias("agb");
	auto g21 = g2.add_group().name("app_group_b1").alias("agb1");
	g21.add_command(c4);
	auto g211 = g2.add_group().name("g211");
	g211.add_command(c3);
	g211.add_command(c4);
	auto c5 = g211.add_command().bind("cmd5", "c5", "c5 helper info.",
		std::bind(&Manager::cmd5, &GetManager(),
		std::placeholders::_1, std::placeholders::_2));
	auto c6 = g211.add_command().bind("cmd6", "c6", "c6 helper info.",
		std::bind(&Manager::cmd6, &GetManager(),
		std::placeholders::_1, std::placeholders::_2));
	auto g22 = g2.add_group().name("g22");
	auto c9 = g22.add_command().bind<Cmd9>("c9 helper info.");

	// g3.c1\2\3\4\5\6\7\8\9
	auto g3 = eco::App::home().add_group().name("g3");
	g3.add_command(c1);
	g3.add_command(c2);
	g3.add_command(c3);
	g3.add_command(c4);
	g3.add_command(c5);
	g3.add_command(c6);
	g3.add_command().bind<Cmd7>("c7 helper info.");
	g3.add_command().bind<Cmd8>("c8 helper info.");;
	g3.add_command(c9);

	Group app_g = eco::App::home().name("cmd");
	app_g.add_command(c1);
	app_g.add_command(c2);
	app_g.add_command(c3);
	app_g.add_command(c4);
	app_g.add_group(g22);
	app_g.add_command(c9);
}


////////////////////////////////////////////////////////////////////////////////
}}}