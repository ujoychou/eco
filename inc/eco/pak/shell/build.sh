#!/bin/bash
################################################################################
#PROJECT: project name. [eco/vision]
#PLATFORM: platform name. [linux-x64/linux-aarch64/win-x64]
#BUILD_TYPE: configure name. [debug/release]
#BUILD_PATH: where project root directory. [git/eco]
#THREADS: compile source code with multiple threads. [1/4/8]
################################################################################

echo "-------------------------------------------------------------------------"
INIT_DIR=`pwd`
declare -l PRJ_CFG=${PROJECT}_${PLATFORM}_${BUILD_TYPE}
echo "== ${PRJ_CFG}"
echo "-------------------------------------------------------------------------"

# package dir
CMAKE_DIR=${SHELL_DIR}/../cmake
CONAN_DIR=${SHELL_DIR}/../conan
TMP_DIR=${BUILD_PATH}/.tmp
BIN_DIR=${BUILD_PATH}/.bin

# conan: install 3rdparty library, from github so on.
mkdir -p ${TMP_DIR}/${PRJ_CFG}
conan install ${CONAN_DIR}/conanfile.txt -if ${TMP_DIR}/${PRJ_CFG}

# cmake: create makefile.
cd ${TMP_DIR}/${PRJ_CFG}
echo "== -> "`pwd`
cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DPLATFORM=${PLATFORM}
# make: build & move to bin.
make -j${THREADS}
mkdir -p ${BIN_DIR}/${PRJ_CFG}

# back to init dir.
cd ${INIT_DIR}
echo "========================================================================"



