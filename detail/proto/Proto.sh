#!/bin/bash

# get "current .sh" filepath.
SHELL_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
API_DIR=${SHELL_DIR}/../../
echo "--shell="${SHELL_DIR}

# set protoc.exe & grpc_cpp_plugin.exe filepath.
PROTOC="/home/ujoy/.conan/data/protobuf/3.17.1/_/_/package/ea576955a126fc70f247bf2ebb9a243c67c507eb/bin/protoc"
GRPC_CPP_PLUGIN=

# generate cpp.
${PROTOC} -I${SHELL_DIR} -I${API_DIR} --cpp_out=${SHELL_DIR} \
Eco.proto \
Monitor.proto

echo "--proto generated."