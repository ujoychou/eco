#!/bin/bash
SHELL_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)

# build eco
PRJ="Eco"
CFG="Debug"
PRJ_DIR=${SHELL_DIR}/../../..
source ${PRJ_DIR}/inc/eco/pak/shell/build.sh