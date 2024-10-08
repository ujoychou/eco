#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/error.hpp>


////////////////////////////////////////////////////////////////////////////////
void test()
{
    const char* url = "10.21.153.55:9050/images/icon/cool.jpg";
    eco::error("login fail, username or password is error.");
    eco::error("get file error: %1 don't have %2 privilege of file %3") % "zhouyu" % 1 % url;
}