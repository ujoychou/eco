cmake_minimum_required(VERSION 3.0.0)
# project name version.
project(EcoMysql VERSION 1.0.1)
# project param for cppxx.cmake
set(PRJ_FMT "lib")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER ${PRJ_DIR}/rx/RxEcoMysqlVersion.h)
set(SRC_DIR ${PRJ_DIR}/src/persist)
set(INS_DIR ${PRJ_DIR}/lib/eco)

# include dir.
include_directories(${PRJ_DIR}/src)
include_directories(${PRJ_DIR}/inc)
# include cpp.
#aux_source_directory(${PRJ_DIR}/src/persist CXX_FILES)
# exclude cpp.
list(APPEND CXX_FILES_EXCLUDE ${PRJ_DIR}/src/persist/Sqlite.cpp)
list(APPEND CXX_FILES_EXCLUDE ${PRJ_DIR}/src/persist/Persist.cpp)
list(APPEND CXX_FILES_EXCLUDE ${PRJ_DIR}/src/persist/CsvSource.cpp)
# include lib.
set(CXX_LIBS libpthread.so)
#list(APPEND CXX_LIBS libEco.so.1.0.2)
# include lib dirs.
list(APPEND CXX_LIB_DIRS ${PRJ_DIR}/lib/eco)

# apply with cpp11
include(${PRJ_DIR}/inc/eco/pak/cmake/cpp11.cmake)
