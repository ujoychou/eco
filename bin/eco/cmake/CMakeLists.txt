cmake_minimum_required(VERSION 3.0.0) 

# project setting: name ver dir fmt
project(eco VERSION ${VERSION})
set(PRJ_FMT "lib")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER ${PRJ_DIR}/inc/eco/version.h)

# src files: directory.
set(SRC_DIRS ${PRJ_DIR}/src)
# src files: exclude cpp.
list(APPEND SRC_DIRS_EXCLUDE  ${SRC_DIRS}/service)
list(APPEND SRC_FILES_EXCLUDE ${SRC_DIRS}/persist/Mysql.cpp)
list(APPEND SRC_FILES_EXCLUDE ${SRC_DIRS}/persist/Sqlite.cpp)
list(APPEND SRC_FILES_EXCLUDE ${SRC_DIRS}/xml/Reader.old.cpp)

# include lib & dir: target private
set(LINK_LIBS libpthread.so libprotobufd.so libz.a)

# install dir
set(INS_DIR ${PRJ_DIR}/lib/eco)

# apply with cpp11
include(${PRJ_DIR}/inc/eco/pak/cmake/c++11.cmake)
