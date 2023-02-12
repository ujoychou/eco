#!/bin/bash
################################################################################
#PRJ: project name. "eco/vision".
#CFG: configure name. "Debug/Release".
#PRJ_DIR: where project root dir. "git/eco".
################################################################################

# start. 
echo "-------------------------------------------------------------------------"
INIT_DIR=`pwd`
declare -l PRJ_CFG=${PRJ}_${CFG}
echo "== ${PRJ_CFG}"
echo "-------------------------------------------------------------------------"

# pak dir: get current ".sh" path.
CMAKE_DIR=${SHELL_DIR}/../cmake
CONAN_DIR=${SHELL_DIR}/../conan
TMP_DIR=${PRJ_DIR}/.tmp
BIN_DIR=${PRJ_DIR}/.bin

# conan: install 3rdparty library.
mkdir -p ${TMP_DIR}/${PRJ_CFG}
conan install ${CONAN_DIR}/conanfile.txt -if ${TMP_DIR}/${PRJ_CFG}

# cmake: create makefile.
cd ${TMP_DIR}/${PRJ_CFG}
echo "== -> "`pwd`
cmake ${CMAKE_DIR}/ -DCMAKE_BUILD_TYPE=${CFG} -DECO_LINUX=1
# make: build & move to bin.
make -j8
mkdir -p ${BIN_DIR}/${PRJ_CFG}
#mv ${TMP_DIR}/${PRJ_CFG}/${PRJ} ${BIN_DIR}/${PRJ_CFG}/

# back to init dir.
cd ${INIT_DIR}
echo "========================================================================"
