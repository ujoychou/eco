cmake_minimum_required(VERSION 3.0.0) 

# project name version.
project(eco_test_unit VERSION ${VERSION})
message("== eco_test_unit version is ${VERSION}")
# project param for cppxx.cmake
set(PRJ_FMT "shared")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER ${PRJ_DIR}/inc/eco/rx/RxVersion.h)
set(CPP_VER 17)
# src files
set(INCLUDE_DIRS ${PRJ_DIR}/include ${PRJ_DIR}/include1)
set(LINK_DIRS ${PRJ_DIR}/lib1/ ${PRJ_DIR}/lib2/)
# link libs
set(SRC_DIRS ${PRJ_DIR}/test/unit ${PRJ_DIR}/test/unit1)
set(SRC_FILES ${PRJ_DIR}/test/unit/1.cpp ${PRJ_DIR}/test/unit/2.cpp)
set(SRC_DIRS_EXCLUDE ${PRJ_DIR}/test/unit2 ${PRJ_DIR}/test/unit1)
set(SRC_FILES_EXCLUDE ${PRJ_DIR}/test/unit/1.cpp ${PRJ_DIR}/test/unit/2.cpp)
set(LINK_LIBS GTest::gtest GTest::gtest_main)

include(${PRJ_DIR}/inc/eco/pak/cmake/c++.cmake)