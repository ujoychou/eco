cmake_minimum_required(VERSION 3.0.0)
# project name version.
project(Eco VERSION 2.0.1)
# project param for cppxx.cmake
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(SRC_DIR ${PRJ_DIR}/prj/src)
set(VER_FILE ${PRJ_DIR}/rx/RxVersion.h)

# include dir.
include_directories(${SRC_DIR})
include_directories(${PRJ_DIR}/..)
include_directories(${SRC_DIR})
# include cpp.
#aux_source_directory(${SRC_DIR1} CXX_FILES)
aux_source_directory(${PRJ_DIR}/detail CXX_FILES)
aux_source_directory(${PRJ_DIR}/detail/proto CXX_FILES)
aux_source_directory(${PRJ_DIR}/detail/sys CXX_FILES)
aux_source_directory(${SRC_DIR} CXX_FILES)
aux_source_directory(${SRC_DIR}/cmd CXX_FILES)
aux_source_directory(${SRC_DIR}/codec CXX_FILES)
aux_source_directory(${SRC_DIR}/config CXX_FILES)
aux_source_directory(${SRC_DIR}/date_time CXX_FILES)
aux_source_directory(${SRC_DIR}/loc CXX_FILES)
aux_source_directory(${SRC_DIR}/log CXX_FILES)
aux_source_directory(${SRC_DIR}/net CXX_FILES)
aux_source_directory(${SRC_DIR}/net/asio CXX_FILES)
aux_source_directory(${SRC_DIR}/net/protocol CXX_FILES)
aux_source_directory(${SRC_DIR}/service CXX_FILES)
aux_source_directory(${SRC_DIR}/sys CXX_FILES)
aux_source_directory(${SRC_DIR}/test CXX_FILES)
aux_source_directory(${SRC_DIR}/web CXX_FILES)
aux_source_directory(${SRC_DIR}/xml CXX_FILES)
aux_source_directory(${SRC_DIR}/persist CXX_FILES)

# exclude cpp.
list(APPEND CXX_FILES_EXCLUDE ${SRC_DIR}/xml/Reader.old.cpp)
# include lib.
set(CXX_LIBS libprotobufd.so libpthread.so)
list(APPEND CXX_LIBS libboost_filesystem.so libboost_thread.so)
list(APPEND CXX_LIBS libz.a)

# apply with cpp11
include(${PRJ_DIR}/pak/cmake/cpp11.cmake)
