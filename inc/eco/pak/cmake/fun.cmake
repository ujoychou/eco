# ---------------------------------------------------------------------
# get absolute path of 'FILES_DIRS'
function(eco_get_absolute_path RESULT)
set(_ABS_ )
foreach(_TEMP_ ${ARGN})
    get_filename_component(_TEMP_ ${_TEMP_} ABSOLUTE)
    list(APPEND _ABS_ ${_TEMP_})
endforeach()
set(${RESULT} ${_ABS_} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------
# include c++ source files in 'SRC_DIRS'
function(eco_files_include __SRC_FILES__)
set(_SRC_FILES_ ${${__SRC_FILES__}})
foreach(_SRC_DIR_ ${ARGN})
    set(_CXX_TEMP )
    file(GLOB_RECURSE _CXX_TEMP ${_SRC_DIR_}/*.cpp)
    list(APPEND _SRC_FILES_ ${_CXX_TEMP})
    file(GLOB_RECURSE _CXX_TEMP ${_SRC_DIR_}/*.cxx)
    list(APPEND _SRC_FILES_ ${_CXX_TEMP})
    file(GLOB_RECURSE _CXX_TEMP ${_SRC_DIR_}/*.cc)
    list(APPEND _SRC_FILES_ ${_CXX_TEMP})
    file(GLOB_RECURSE _CXX_TEMP ${_SRC_DIR_}/*.c)
    list(APPEND _SRC_FILES_ ${_CXX_TEMP})
endforeach()
set(${__SRC_FILES__} ${_SRC_FILES_} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------
# exclude c++ source files in 'SRC_DIRS_EXCLUDE'
function(eco_files_exclude __SRC_FILES__)
set(_SRC_FILES_ ${${__SRC_FILES__}})
foreach(_SRC_DIR_ ${ARGN})
    set(_CXX_TEMP )
    file(GLOB_RECURSE _CXX_TEMP ${_SRC_DIR_}/*.cpp)
    list(REMOVE_ITEM _SRC_FILES_ ${_CXX_TEMP})
    file(GLOB_RECURSE _CXX_TEMP ${_SRC_DIR_}/*.cxx)
    list(REMOVE_ITEM _SRC_FILES_ ${_CXX_TEMP})
    file(GLOB_RECURSE _CXX_TEMP ${_SRC_DIR_}/*.cc)
    list(REMOVE_ITEM _SRC_FILES_ ${_CXX_TEMP})
    file(GLOB_RECURSE _CXX_TEMP ${_SRC_DIR_}/*.c)
    list(REMOVE_ITEM _SRC_FILES_ ${_CXX_TEMP})
endforeach()
set(${__SRC_FILES__} ${_SRC_FILES_} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------
# exclude c++ source files in 'SRC_DIRS_EXCLUDE'
function(eco_project __PRJ_OUTPUT__ PRJ_FMT INSTALL_DIR)
set(_PRJ_OUTPUT_ )
if(${PRJ_FMT} STREQUAL "exe")
    add_executable(${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} 
        PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${INSTALL_DIR})
    set(_PRJ_OUTPUT_ ${INSTALL_DIR}/${PROJECT_NAME})
elseif(${PRJ_FMT} STREQUAL "shared")
    add_library(${PROJECT_NAME} SHARED)
    set(_SUFFIX_ ".so.${PROJECT_VERSION}")
    set_target_properties(${PROJECT_NAME} 
        PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${INSTALL_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ${_SUFFIX_})
    set(_PRJ_OUTPUT_ ${INSTALL_DIR}/${PROJECT_NAME}${_SUFFIX_})
elseif(${PRJ_FMT} STREQUAL "static")
    add_library(${PROJECT_NAME} STATIC)
    set_target_properties(${PROJECT_NAME} 
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${INSTALL_DIR})
    set(_PRJ_OUTPUT_ ${INSTALL_DIR}/${PROJECT_NAME})
endif()
set(${__PRJ_OUTPUT__} ${_PRJ_OUTPUT_} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------
# get absolute path of 'FILES_DIRS'
function(eco_messages TITLE)
foreach(_MSG_ ${ARGN})
    message("${TITLE}${_MSG_}")
endforeach()
endfunction()