#!/bin/bash

#inputs
MODE=$1 #full build, run tool: pair, pptr, log
TEST_NAME=$2 

#directories
BASE_DIR=$(dirname $(realpath "$0"))
BUILD_DIR="${BASE_DIR}/dfbuild"
TEST_DIR="${BASE_DIR}/test"
SINGLE_FILE_REPO=${TEST_DIR}/single_file

LLVM_BASE_DIR=~/llvm_compiler8/
COMPILER_DIR=${LLVM_BASE_DIR}/bin
LLVM_DIR=${LLVM_BASE_DIR}/build/lib/cmake/llvm/

#variables
TOOL_NAME=$MODE

init_build(){
	cd ${BASE_DIR}
	mkdir ${BUILD_DIR}
	cd ${BUILD_DIR}
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
		-DLLVM_DIR=${LLVM_DIR} \
		-DCMAKE_C_COMPILER=${COMPILER_DIR}/clang \ 
		-DCMAKE_CXX_COMPILER=${COMPILER_DIR}/clang++ ..
  cd ${BASE_DIR}
}

run_make(){
	cd ${BUILD_DIR}
	make -j$(nproc)
	cd ${BASE_DIR}
}

remove_build(){
	cd ${BASE_DIR}
	rm -r $BUILD_DIR
}

run_fullbuild(){
    remove_build
    init_build
    run_make
}

create_ir(){
	cd ${SINGLE_FILE_REPO}
	echo "${SINGLE_FILE_REPO}"
	clang++ -g -c -O3 -emit-llvm ${TEST_NAME}.cpp -o ${TEST_NAME}.bc
	clang++ -g -c -O3 -emit-llvm -S ${TEST_NAME}.cpp -o ${TEST_NAME}.ll
	cd ${BASE_DIR}
}

run_tool(){
    $BUILD_DIR/bin/$TOOL_NAME ${SINGLE_FILE_REPO}/${TEST_NAME}.bc
}

#commands----------------------------------------------------
if [ "$MODE" == "pair" ] || [ "$MODE" == "pptr" ] || [ "$MODE" == "log" ];then
	run_make
	run_tool
elif [ "$MODE" == "make" ] ;then
	run_make
elif [ "$MODE" == "build" ] ;then
  run_fullbuild
elif [ "$MODE" == "ir" ] ;then
	create_ir
else
	echo "pair/log/pptr, build, ir"
fi
#commands----------------------------------------------------