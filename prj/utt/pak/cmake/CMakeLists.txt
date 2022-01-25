cmake_minimum_required(VERSION 3.0.0)
# project name version.
project(Utt VERSION 2.0.1)
# project param for cppxx.cmake
set(ECO_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
set(SRC_DIR  ${ECO_DIR}/prj/utt)
set(VER_FILE ${SRC_DIR}/Version.h)
set(PRJ_DIR  ${ECO_DIR})
set(ECO_SRC  ${ECO_DIR}/prj/src)

# include dir.
include_directories(${ECO_DIR}/..)
include_directories(${SRC_DIR})
include_directories(${ECO_SRC})
# include cpp.
file(GLOB_RECURSE SRC_DIR_FILES ${ECO_DIR}/detail/*.cpp ${ECO_DIR}/detail/*.cc ${ECO_DIR}/detail/*.c)
#set(CXX_FILES "${SRC_DIR_FILES}")
#aux_source_directory(${ECO_DIR}/detail CXX_FILES)
list(APPEND CXX_FILES ${SRC_DIR}/Cast.cpp)
#list(APPEND CXX_FILES ${SRC_DIR}/Object.cpp)
#list(APPEND CXX_FILES ${SRC_DIR}/Memory.cpp)
#list(APPEND CXX_FILES ${SRC_DIR}/String.cpp)
#list(APPEND CXX_FILES ${SRC_DIR}/Number.cpp)
#list(APPEND CXX_FILES ${SRC_DIR}/Rtype.cpp)
# exclude cpp.
list(APPEND CXX_FILES_EXCLUDE ${SRC_DIR}/xml/Reader.old.cpp)
# include lib.
set(CXX_LIBS libprotobufd.so libpthread.so)
list(APPEND CXX_LIBS libboost_filesystem.so libboost_thread.so libgtestd.a libgtest_maind.a)

# apply with cpp11
include(${ECO_DIR}/pak/cmake/cpp11.cmake)

#gtest_discover_tests(${PROJECT_NAME}, .noArg, AUTO)