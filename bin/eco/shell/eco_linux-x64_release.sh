#!/bin/bash
SHELL_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)

# build eco
PROJECT="eco"
PLATFORM="linux-x64"
BUILD_TYPE="release"
BUILD_PATH=${SHELL_DIR}/../../..
THREADS=8
source ${PATH}/inc/eco/pak/shell/build.sh