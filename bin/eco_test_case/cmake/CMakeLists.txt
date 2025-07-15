cmake_minimum_required(VERSION 3.0.0) 

# project
project(eco_test_case VERSION ${VERSION})
set(PRJ_FMT "exe")
set(PRJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(PRJ_VER ${PRJ_DIR}/inc/eco/rx/RxVersion.h)
set(CPP_VER 17)
# include & link
# source
set(SRC_FILES
    ${PRJ_DIR}/test/case/freelist_app.cpp)

include(${PRJ_DIR}/inc/eco/pak/cmake/c++.cmake)