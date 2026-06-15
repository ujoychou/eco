cmake_minimum_required(VERSION 3.0.0) 

# project name version.
project(eco_mysql VERSION 1.0.1)
# project param for cppxx.cmake
set(PRJ_FMT "lib")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER_FILE ${PRJ_DIR}/src/eco/persist/MysqlVersion.h)

# src files: directory.
set(SRC_DIRS ${PRJ_DIR}/src/persist)
# src files: include cpp.
aux_source_directory(${PRJ_DIR}/src/persist SRC_FILES)
# src files: exclude cpp.
list(APPEND SRC_FILES_EXCLUDE ${PRJ_DIR}/src/persist/Sqlite.cpp)
list(APPEND SRC_FILES_EXCLUDE ${PRJ_DIR}/src/persist/Persist.cpp)
list(APPEND SRC_FILES_EXCLUDE ${PRJ_DIR}/src/persist/CsvSource.cpp)
# src files: exclude dir.

# include dir: target private
set(INC_DIRS ${PRJ_DIR}/src) 
# include dir: global
include_directories(${PRJ_DIR}/src)
# include lib & dir: target private
set(LINK_LIBS libpthread.so)
#list(APPEND LINK_LIBS ${PRJ_DIR}/.lib/eco/libEco.so.1.0.2)
set(LINK_LIB_DIRS ${PRJ_DIR}/.lib/eco)
# include lib & dir: global
link_directories(${PRJ_DIR}/.lib/eco)
link_libraries(libpthread.so)
#link_libraries(${PRJ_DIR}/.lib/eco/libEco.so.1.0.2)

# install dir
set(INS_DIR ${PRJ_DIR}/.lib/eco)

# apply with cpp11
include(${PRJ_DIR}/inc/eco/pak/cmake/cpp11.cmake)
