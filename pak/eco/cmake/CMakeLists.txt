cmake_minimum_required(VERSION 3.0.0)
# project name version.
project(Eco VERSION 1.0.2)
# project param for cppxx.cmake
set(PRJ_FMT "lib")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER ${PRJ_DIR}/rx/RxEcoVersion.h)
set(SRC_DIR ${PRJ_DIR}/src)
set(INS_DIR ${PRJ_DIR}/lib/eco)

# include dir.
include_directories(${PRJ_DIR}/inc)
# include cpp.
# exclude cpp.
set(CXX_DIRS_EXCLUDE ${SRC_DIR}/service)
list(APPEND CXX_FILES_EXCLUDE ${SRC_DIR}/xml/Reader.old.cpp)
list(APPEND CXX_FILES_EXCLUDE ${SRC_DIR}/persist/Mysql.cpp)
list(APPEND CXX_FILES_EXCLUDE ${SRC_DIR}/persist/Sqlite.cpp)
# include lib.
set(CXX_LIBS libprotobufd.so libpthread.so)
list(APPEND CXX_LIBS libboost_filesystem.so libboost_thread.so)
list(APPEND CXX_LIBS libz.a)

# apply with cpp11
include(${PRJ_DIR}/inc/eco/pak/cmake/cpp11.cmake)
