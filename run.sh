#!/bin/bash

#inputs
MODE=$1 #build, pair, dur, log
TEST_NAME=$2 #under single file

#directories
BASE_DIR=$(dirname $(realpath "$0"))
BUILD_DIR="${BASE_DIR}/dfbuild"
TEST_DIR="${BASE_DIR}/test"
SINGLE_FILE_REPO=${TEST_DIR}/single_file

LLVM_BASE_DIR=~/llvm_compiler8
COMPILER_DIR=${LLVM_BASE_DIR}/bin
LLVM_DIR=${LLVM_BASE_DIR}/build/lib/cmake/llvm/

#variables
TOOL_NAME=$MODE

init_build(){
	cd ${BASE_DIR}
	mkdir ${BUILD_DIR}
	cd ${BUILD_DIR}
	cmake -v -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
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
	if [ -d "$BUILD_DIR" ]; then rm -r $BUILD_DIR; fi
}

run_fullbuild(){
    remove_build
    init_build
    run_make
}

create_ir(){
	cd ${SINGLE_FILE_REPO}
	make all -j$(nproc)
	cd ${BASE_DIR}
}

create_ll(){
	cd ${SINGLE_FILE_REPO}
	make ll -j$(nproc)
	cd ${BASE_DIR}
}

clean_ir(){
	cd ${SINGLE_FILE_REPO}
	make clean
	cd ${BASE_DIR}
}

#--debug-pass=Structure
run_tool(){
		opt \
-load $BUILD_DIR/lib/lib${TOOL_NAME}.so -${TOOL_NAME} \
${SINGLE_FILE_REPO}/${TEST_NAME}.bc > /dev/null
}

checkers=("pair" "dur" "log" "exp" "cons" "simp" "alias" "parse")

function contains() {
    local n=$#
    local value=${!n}
    for ((i=1;i < $#;i++)) {
        if [ "${!i}" == "${value}" ]; then
            echo "y"
            return 0
        fi
    }
    echo "n"
    return 1
}

#commands----------------------------------------------------
if [ $(contains "${checkers[@]}" "$MODE") == "y" ] ;then
	create_ir
	run_make
	run_tool
elif [ "$MODE" == "make" ] ;then
	run_make
elif [ "$MODE" == "build" ] ;then
  run_fullbuild
elif [ "$MODE" == "ir" ] ;then
	create_ir
elif [ "$MODE" == "ll" ] ;then
	create_ll
elif [ "$MODE" == "rem_ir" ] ;then
	clean_ir
else
	echo "pair, dur, log, exp, cons, simp, alias, parse, make, build, ir, rem_ir"
fi
#commands----------------------------------------------------