################################################################################
# PLATFORM
# COMPILER

# PRJ_DIR: project directory, which include: bin/doc/inc/src/test, .tmp/.bin
# PRJ_VER: the output version file.
# CPP_VER: 
# INSTALL_DIR:

# INCLUDE_DIRS
# LINK_LIBS
# LINK_DIRS

# SRC_DIRS: dedicated the source files to build.
# SRC_FILES
# SRC_DIRS_EXCLUDE
# SRC_FILES_EXCLUDE
################################################################################
include(${CMAKE_CURRENT_LIST_DIR}/fun.cmake)

# platform
if (${PLATFORM} STREQUAL "linux-x64")
    add_definitions(-DECO_LINUX)
elseif (${PLATFORM} STREQUAL "linux-aarch64")
    add_definitions(-DECO_LINUX)
elseif (${PLATFORM} STREQUAL "win-x64")
    add_definitions(-DECO_WIN32)
elseif (${PLATFORM} STREQUAL "win-aarch64")
    add_definitions(-DECO_WIN32)
else()
    message(WARNING "unknown arch platform: ${PLATFORM}")
endif()

# project full name & project version template file.
# exp: [eco-linux-x64-debug-3.2.0]
set(PRJ_CFG ${PROJECT_NAME}_${PLATFORM}_${CMAKE_BUILD_TYPE}_${PROJECT_VERSION})
string(TOLOWER ${PRJ_CFG} PRJ_CFG)
if(PRJ_VER)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.tpl.h ${PRJ_VER})
endif()
message("---------------------------------------------------------------------")

# build directory
set(TMP_DIR ${PRJ_DIR}/.tmp/${PRJ_CFG})
set(BIN_DIR ${PRJ_DIR}/.bin/${PRJ_CFG})
if (NOT INSTALL_DIR)
    set(INSTALL_DIR ${BIN_DIR})
endif()

# get absolute path
get_filename_component(PRJ_DIR ${PRJ_DIR} ABSOLUTE)
get_filename_component(TMP_DIR ${TMP_DIR} ABSOLUTE)
get_filename_component(BIN_DIR ${BIN_DIR} ABSOLUTE)
get_filename_component(INSTALL_DIR ${INSTALL_DIR} ABSOLUTE)
eco_get_absolute_path(SRC_DIRS ${SRC_DIRS})
eco_get_absolute_path(SRC_DIRS_EXCLUDE ${SRC_DIRS_EXCLUDE})
eco_get_absolute_path(SRC_FILES ${SRC_FILES})
eco_get_absolute_path(SRC_FILES_EXCLUDE ${SRC_FILES_EXCLUDE})
eco_get_absolute_path(INCLUDE_DIRS ${INCLUDE_DIRS})
eco_get_absolute_path(LINK_DIRS ${LINK_DIRS})
message("== PRJ_DIR:${PRJ_DIR}")
message("==.BIN_DIR:${BIN_DIR}")
message("==.TMP_DIR:${TMP_DIR}")
message("== INSTALL_DIR:${INSTALL_DIR}")
message("== CMAKE_DIR:${CMAKE_CURRENT_SOURCE_DIR}")
message("== CMAKE_BIN_DIR:${CMAKE_CURRENT_BINARY_DIR}")
message("---------------------------------------------------------------------")


################################################################################
# c++ compiler: c++
set(CMAKE_CXX_STANDARD ${CPP_VER})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# c++ compiler: "compile_commands.json"
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# c++ compiler
if (${COMPILER} STREQUAL "gcc")
    add_definitions(-DECO_GCC)
    include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
elseif (${COMPILER} STREQUAL "msvc")
    add_definitions(-DECO_MSVC)
    include(${CMAKE_CURRENT_LIST_DIR}/msvc.cmake)
    option(MSVC_PCH "msvc precompile header file in windows." OFF)
elseif (${COMPILER} STREQUAL "clang")
    add_definitions(-DECO_CLANG)
    include(${CMAKE_CURRENT_LIST_DIR}/clang.cmake)
else()
    message(WARNING "unknown c++ compiler: ${COMPILER}")
endif()
# c++ compiler: logging
message("== CXX_VER:c++${CPP_VER}")
message("== CXX_COMPILER:${COMPILER}")
message("== CXX_DEBUG:${CMAKE_CXX_FLAGS_DEBUG}")
message("== CXX_RELEASE:${CMAKE_CXX_FLAGS_RELEASE}")
message("== CXX_FLAGS:${CMAKE_CXX_FLAGS}")
message("== CXX_COMPILE_COMMANDS:${TMP_DIR}/compile_commands.json")
message("---------------------------------------------------------------------")


################################################################################
# c++ target: output name. (exe/so/a)
eco_project(OUTPUT_FILE ${PRJ_FMT} ${INSTALL_DIR})
# c++ target: inc & lib & install
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PRJ_DIR}/inc)
target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBS})
target_link_directories(${PROJECT_NAME} PRIVATE ${LINK_DIRS})
# c++ target: inc & lib of conan
if(EXISTS ${TMP_DIR}/conandeps_legacy.cmake)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${TMP_DIR})
    include(${TMP_DIR}/conandeps_legacy.cmake)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CONANDEPS_LEGACY})
endif()
eco_messages("== SRC_DIR:" ${SRC_DIRS})
eco_messages("== INCLUDE_DIR:" ${INCLUDE_DIRS})
eco_messages("== LINK_DIR:" ${LINK_DIRS})
message("== LINK_LIBS:${LINK_LIBS}")
message("== LINK_LIBS_CONAN:${CONANDEPS_LEGACY}")
message("---------------------------------------------------------------------")
eco_messages("== SRC_DIR:" ${SRC_DIRS})
eco_messages("== SRC_FILE:" ${SRC_FILES})
eco_messages("== SRC_DIR_EXCLUDE:" ${SRC_DIRS_EXCLUDE})
eco_messages("== SRC_FILE_EXCLUDE:" ${SRC_FILES_EXCLUDE})
# c++ target: source files
eco_files_include(SRC_FILES ${SRC_DIRS})
eco_files_exclude(SRC_FILES ${SRC_DIRS_EXCLUDE})
list(REMOVE_ITEM SRC_FILES ${SRC_FILES_EXCLUDE})
target_sources(${PROJECT_NAME} PRIVATE ${SRC_FILES})
# c++ target: inc & lib & install.
eco_messages("== SOURCE:" ${SRC_FILES})
eco_messages("== OUTPUT:" ${OUTPUT_FILE})
message("---------------------------------------------------------------------")