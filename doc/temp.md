// import eco/log.
#undef ECO_API
#define ECO_API __declspec(dllimport)
#include <eco/log/Log.h>
#pragma comment(lib, "eco.lib")
#undef ECO_API
#define ECO_API __declspec(dllexport)



编译eco
编译eco_mysql
编译eco_sqlite

开发项目与工程。