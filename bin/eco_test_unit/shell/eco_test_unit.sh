#!/bin/bash
SHELL_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)

# build eco
PROJECT="eco_test_unit"
VERSION=1.3.1
# Debug Release RelWithDebInfo
BUILD_TYPE=Debug
BUILD_PATH=${SHELL_DIR}/../../..
# Set the platform and compiler
PLATFORM_HOST=linux-x64
# Cross-compile when different from host
PLATFORM_BUILD=linux-x64
COMPILER=gcc
THREADS=1
source ${BUILD_PATH}/inc/eco/pak/shell/build.sh