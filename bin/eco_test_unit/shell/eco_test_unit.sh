#!/bin/bash
SHELL_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)

# build eco
PROJECT="eco_test_unit"
VERSION=1.3.1
BUILD_TYPE=release
BUILD_PATH=${SHELL_DIR}/../../..
PLATFORM_HOST=linux-x64
PLATFORM_BUILD=linux-x64
THREADS=8
source ${BUILD_PATH}/inc/eco/pak/shell/build.sh