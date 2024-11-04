cmake_minimum_required(VERSION 3.0.0) 

# project
project(eco_test_unit VERSION ${VERSION})
set(PRJ_FMT "exe")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER ${PRJ_DIR}/inc/eco/rx/RxVersion.h)
set(CPP_VER 17)
# include & link
set(LINK_LIBS GTest::gtest GTest::gtest_main)
# source
set(SRC_FILES ${PRJ_DIR}/test/unit/test_string.cpp)

include(${PRJ_DIR}/inc/eco/pak/cmake/c++.cmake)