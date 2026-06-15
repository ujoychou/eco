cmake_minimum_required(VERSION 3.0.0) 

# project setting: name ver dir fmt
project(eco_test_unit VERSION ${VERSION})
set(PRJ_FMT "exe")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER ${PRJ_DIR}/test/unit/version.h)
set(CPP_VER 17)

# src files
set(SRC_FILES
    ${PRJ_DIR}/test/unit/cast/test_cast_integer_2.cpp
    ${PRJ_DIR}/test/unit/cast/test_cast_integer_2_ai.cpp
    ${PRJ_DIR}/test/unit/cast/test_cast_integer_1.cpp
    ${PRJ_DIR}/test/unit/cast/test_cast_integer_1_ai.cpp
    ${PRJ_DIR}/test/unit/cast/ryu_64.cc
    ${PRJ_DIR}/src/log/config.cpp
    #${PRJ_DIR}/src/log/log.cpp
    ${PRJ_DIR}/test/unit/test_elog.cpp
    ${PRJ_DIR}/src/thread.cpp
    ${PRJ_DIR}/src/rtti.cpp
    ${PRJ_DIR}/src/plugin.cpp
    ${PRJ_DIR}/test/unit/plugin/plugin_logger.cpp
    ${PRJ_DIR}/test/unit/plugin/test_plugin.cpp
    ${PRJ_DIR}/test/unit/lockfree/test_queue.cpp
    ${PRJ_DIR}/test/unit/sync/test_spinlock.cpp
    ${PRJ_DIR}/test/unit/test_number.cpp
    ${PRJ_DIR}/test/unit/test_except.cpp
    ${PRJ_DIR}/test/unit/lockfree/test_freepool.cpp
    ${PRJ_DIR}/test/unit/lockfree/test_spanpool.cpp
    ${PRJ_DIR}/test/unit/lockfree/test_freelist.cpp)

# include lib & dir: target private
find_package(GTest)
set(INC_DIRS ${PRJ_DIR}/test/unit)
set(LINK_LIBS GTest::gtest GTest::gtest_main)

# apply with cpp11
include(${PRJ_DIR}/inc/eco/pak/cmake/c++.cmake)