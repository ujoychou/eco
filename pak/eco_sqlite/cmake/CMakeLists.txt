cmake_minimum_required(VERSION 3.0.0) 

# project name version.
project(EcoSqlite VERSION 1.0.1)
# project param for cppxx.cmake
set(PRJ_FMT "lib")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

# src files: directory.
set(SRC_DIRS ${PRJ_DIR}/src/persist)
# src files: exclude cpp.
list(APPEND SRC_FILES_EXCLUDE ${PRJ_DIR}/src/persist/Mysql.cpp)
list(APPEND SRC_FILES_EXCLUDE ${PRJ_DIR}/src/persist/Persist.cpp)
list(APPEND SRC_FILES_EXCLUDE ${PRJ_DIR}/src/persist/CsvSource.cpp)

# include dir: target private
set(INC_DIRS ${PRJ_DIR}/src) 
# include lib & dir: target private
set(LINK_LIBS libpthread.so)
set(LINK_LIB_DIRS ${PRJ_DIR}/lib/eco)

# install dir
set(INS_DIR ${PRJ_DIR}/lib/eco)

# apply with cpp11
include(${PRJ_DIR}/inc/eco/pak/cmake/cpp11.cmake)
