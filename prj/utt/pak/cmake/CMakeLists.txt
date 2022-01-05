cmake_minimum_required(VERSION 3.0.0)
# project name version.
project(EcoUtt VERSION 2.0.1)
# project param for cppxx.cmake
set(ECO_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
set(SRC_DIR  ${ECO_DIR}/prj/utt)
set(VER_FILE ${SRC_DIR}/Version.h)
set(PRJ_DIR  ${ECO_DIR})

# include dir.
include_directories(${ECO_DIR}/..)
include_directories(${SRC_DIR})
# include cpp.
#aux_source_directory(${SRC_DIR} CXX_FILES)
list(APPEND CXX_FILES ${SRC_DIR}/Cast.cpp)
list(APPEND CXX_FILES ${SRC_DIR}/Def.cpp)
list(APPEND CXX_FILES ${SRC_DIR}/Memory.cpp)
list(APPEND CXX_FILES ${SRC_DIR}/String.cpp)
list(APPEND CXX_FILES ${SRC_DIR}/Number.cpp)
list(APPEND CXX_FILES ${SRC_DIR}/Rtype.cpp)
# exclude cpp.
list(APPEND CXX_FILES_EXCLUDE ${SRC_DIR}/xml/Reader.old.cpp)
# include lib.
set(CXX_LIBS libprotobufd.so libpthread.so)
list(APPEND CXX_LIBS libboost_filesystem.so libboost_thread.so gtest gtest_main)

# apply with cpp11
include(${ECO_DIR}/pak/cmake/cpp11.cmake)
