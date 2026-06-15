#!/bin/bash
################################################################################
# PROJECT: project name. [eco/vision]
# VERSION: project version. [1.3.1]
# BUILD_TYPE: [debug/release]
# BUILD_PATH: build output directory. [git/eco]
# PLATFORM_HOST: 
# PLATFORM_BUILD: target platform of build. [linux_x64/linux_aarch64/win_x64]
# COMPILER: [gcc/msvc/clang]
# THREADS: compile source code with multiple threads. [1/4/8]
################################################################################


# exp: eco-linux_x64-debug-3.2.0
echo "-------------------------------------------------------------------------"
INIT_DIR=`pwd`
PLATFORM=${PLATFORM_BUILD}
declare -l PRJ_CFG=${PROJECT}-${PLATFORM}-${BUILD_TYPE}-${VERSION}
echo "== ${PRJ_CFG}"
echo "== target(${PLATFORM_BUILD})"
echo "== current(${PLATFORM_HOST})"
echo "-------------------------------------------------------------------------"

# package dir
CMAKE_DIR=${SHELL_DIR}/../cmake
CONAN_DIR=${SHELL_DIR}/../conan
TMP_DIR=${BUILD_PATH}/.tmp
BIN_DIR=${BUILD_PATH}/.bin

# conan: install 3rdparty library, from github so on.
mkdir -p ${TMP_DIR}/${PRJ_CFG}
PLATFORM_HOST="${PLATFORM_HOST//_/-}"
PLATFORM_BUILD="${PLATFORM_BUILD//_/-}"
conan install ${CONAN_DIR}/conanfile.txt -of ${TMP_DIR}/${PRJ_CFG} \
--profile:host=${CONAN_DIR}/profile_${PLATFORM_HOST} \
--profile:build=${CONAN_DIR}/profile_${PLATFORM_BUILD}

# cmake: create makefile.
cd ${TMP_DIR}/${PRJ_CFG}
cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DVERSION=${VERSION} \
-DPLATFORM=${PLATFORM} -DCOMPILER=${COMPILER} -DPRJ_CFG=${PRJ_CFG}
# make: build & move to bin.
make -j${THREADS}
mkdir -p ${BIN_DIR}/${PRJ_CFG}

# back to init dir.
cd ${INIT_DIR}
echo "========================================================================"
