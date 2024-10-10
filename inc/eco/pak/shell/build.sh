#!/bin/bash
################################################################################
#PROJECT: project name. [eco/vision]
#VERSION: project version. [1.3.1]
#BUILD_TYPE: configure name. [debug/release]
#BUILD_PATH: where project root directory. [git/eco]
#PLATFORM_HOST: platform name. [linux-x64/linux-aarch64/win-x64]
#PLATFORM_BUILD: 
#THREADS: compile source code with multiple threads. [1/4/8]
################################################################################

echo "-------------------------------------------------------------------------"
INIT_DIR=`pwd`
declare -l PRJ_CFG=${PROJECT}_${PLATFORM_HOST}_${BUILD_TYPE}_${VERSION}
echo "== ${PRJ_CFG}"
echo "-------------------------------------------------------------------------"

# package dir
CMAKE_DIR=${SHELL_DIR}/../cmake
CONAN_DIR=${SHELL_DIR}/../conan
TMP_DIR=${BUILD_PATH}/.tmp
BIN_DIR=${BUILD_PATH}/.bin

# conan: install 3rdparty library, from github so on.
mkdir -p ${TMP_DIR}/${PRJ_CFG}
conan install ${CONAN_DIR}/conanfile.txt -of ${TMP_DIR}/${PRJ_CFG} \
--profile:host=${CONAN_DIR}/profile_${PLATFORM_HOST} \
--profile:build=${CONAN_DIR}/profile_${PLATFORM_BUILD}

# cmake: create makefile.
cd ${TMP_DIR}/${PRJ_CFG}
echo "== -> "`pwd`
cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DPLATFORM=${PLATFORM_HOST} \
-DVERSION=${VERSION}
# make: build & move to bin.
make -j${THREADS}
mkdir -p ${BIN_DIR}/${PRJ_CFG}

# back to init dir.
cd ${INIT_DIR}
echo "========================================================================"
