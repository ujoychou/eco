#!/bin/bash
SHELL_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)

# build eco
PROJECT="eco"
PROJECT_DIR=${SHELL_DIR}/../../..
CONFIG="debug"
PLATFORM="linux-x64"
THREADS=8
source ${PATH}/inc/eco/pak/shell/build.sh