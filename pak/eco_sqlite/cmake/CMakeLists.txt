cmake_minimum_required(VERSION 3.0.0)
# project name version.
project(Eco VERSION 1.0.1)
# project param for cppxx.cmake
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(SRC_DIR ${PRJ_DIR}/prj/src)
set(PRJ_VER ${PRJ_DIR}/rx/RxEcoSqliteVersion.h)

# include dir.
include_directories(${SRC_DIR})
include_directories(${PRJ_DIR}/..)
include_directories(${SRC_DIR})
# include cpp.
list(APPEND CXX_FILES ${SRC_DIR}/persist/Mysql.cpp)

# exclude cpp.
# include lib.
set(CXX_LIBS libpthread.so)
list(APPEND CXX_LIBS libEco.so)

# apply with cpp11
include(${PRJ_DIR}/pak/cmake/cpp11.cmake)
