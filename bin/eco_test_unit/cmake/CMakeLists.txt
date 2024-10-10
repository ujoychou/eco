cmake_minimum_required(VERSION 3.0.0) 

# project name version.
project(eco_test_unit VERSION ${VERSION})
message("== eco_test_unit version is ${VERSION}")
# project param for cppxx.cmake
set(PRJ_FMT "shared")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER_FILE ${PRJ_DIR}/inc/eco/rx/RxVersion.h)

# src files: directory.
set(SRC_DIRS ${PRJ_DIR}/test/unit)

# include lib & dir: target private
find_package(GTest REQUIRED)
set(LINK_LIBS gtest::gtest gtest::main)

# install dir
#set(INS_DIR ${PRJ_DIR}/lib/eco)

# apply with cpp11
include(${PRJ_DIR}/inc/eco/pak/cmake/c++11.cmake)
