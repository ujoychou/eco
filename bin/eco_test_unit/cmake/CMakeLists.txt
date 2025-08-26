cmake_minimum_required(VERSION 3.0.0) 

# project
project(eco_test_unit VERSION ${VERSION})
set(PRJ_FMT "exe")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER ${PRJ_DIR}/inc/eco/rx/RxVersion.h)
set(CPP_VER 17)
# include & link
set(LINK_LIBS GTest::gtest GTest::gtest_main)
set(INCLUDE_DIRS ${PRJ_DIR}/test/unit)
# source
set(SRC_FILES
    ${PRJ_DIR}/test/unit/cast/test_cast_integer_2.cpp
    ${PRJ_DIR}/test/unit/cast/test_cast_integer_2_ai.cpp
    ${PRJ_DIR}/test/unit/cast/test_cast_integer_1.cpp
    ${PRJ_DIR}/test/unit/cast/test_cast_integer_1_ai.cpp
    ${PRJ_DIR}/test/unit/cast/ryu_64.cc
    ${PRJ_DIR}/test/unit/test_log.cpp
    ${PRJ_DIR}/src/log.cpp
    ${PRJ_DIR}/src/logger.cpp
    ${PRJ_DIR}/src/thread.cpp
    ${PRJ_DIR}/src/rtti.cpp
    ${PRJ_DIR}/src/plugin.cpp
    ${PRJ_DIR}/test/unit/plugin/plugin_logger.cpp
    ${PRJ_DIR}/test/unit/plugin/test_plugin.cpp
    ${PRJ_DIR}/test/unit/lockfree/test_queue.cpp
    ${PRJ_DIR}/test/unit/sync/test_spinlock.cpp
    ${PRJ_DIR}/test/unit/test_number.cpp
    ${PRJ_DIR}/test/unit/test_error.cpp
    ${PRJ_DIR}/test/unit/lockfree/test_freepool.cpp
    ${PRJ_DIR}/test/unit/lockfree/test_spanpool.cpp
    ${PRJ_DIR}/test/unit/lockfree/test_freelist.cpp)

include(${PRJ_DIR}/inc/eco/pak/cmake/c++.cmake)