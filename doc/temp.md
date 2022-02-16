// import eco/log.
#undef ECO_API
#define ECO_API __declspec(dllimport)
#include <eco/log/Log.h>
#pragma comment(lib, "eco.lib")
#undef ECO_API
#define ECO_API __declspec(dllexport)