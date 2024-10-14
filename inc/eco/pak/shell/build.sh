#!/bin/bash
################################################################################
# PROJECT: project name. [eco/vision]
# VERSION: project version. [1.3.1]
# BUILD_TYPE: [debug/release]
# BUILD_PATH: build output directory. [git/eco]
# PLATFORM_HOST: [linux-x64/linux-aarch64/win-x64]
# PLATFORM_BUILD: 
# COMPILER: [gcc/msvc/clang]
# THREADS: compile source code with multiple threads. [1/4/8]
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
cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DVERSION=${VERSION} \
-DPLATFORM=${PLATFORM_HOST} -DCOMPILER=${COMPILER}
# make: build & move to bin.
make -j${THREADS}
mkdir -p ${BIN_DIR}/${PRJ_CFG}

# back to init dir.
cd ${INIT_DIR}
echo "========================================================================"
