#!/bin/bash
SHELL_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)

# build eco
PRJ="EcoMysql"
CFG="Debug"
PRJ_DIR=${SHELL_DIR}/../../..
ECO_DIR=${PRJ_DIR}/inc/eco/
source ${ECO_DIR}/pak/shell/build.sh