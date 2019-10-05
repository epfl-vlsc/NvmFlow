#!/bin/bash

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

#variables ---------------------------------------------------------------------------------------

#inputs
MODE=$1 #mode or checker
TEST_NAME=$2 #test name

LLVM_BASE_DIR=~/llvm_compiler8
COMPILER_DIR=${LLVM_BASE_DIR}/bin
LLVM_DIR=${LLVM_BASE_DIR}/build/lib/cmake/llvm/

BASE_DIR=$(dirname $(realpath "$0"))
BUILD_DIR="${BASE_DIR}/dfbuild"

#benchmark directories
SINGLE_DIR="${BASE_DIR}/test/single_file"
BENCH_DIR=${BASE_DIR}/../NvmBenchmarks

CHECKERS=("pair" "dur" "log")

ALL_CHECKERS=("pair" "dur" "log" "exp" "cons" "simp" "alias" "parse")

BENCHMARKS=("echo" "nstore" "nvml" "pmfs" "pmgd" "splitfs")

# functions ----------------------------------------------------------------------------------

init_test(){
	TOOL_NAME=$MODE
	if [ $(contains "${BENCHMARKS[@]}" "$TEST_NAME") == "y" ] ;then
		TEST_FILE=${BENCH_DIR}/benchmarks/${TEST_NAME}.bc
		TEST_DIR=${BENCH_DIR}
	else
		TEST_FILE=${SINGLE_DIR}/${TEST_NAME}.bc
		TEST_DIR=${SINGLE_DIR}
	fi
}

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

#--debug-pass=Structure
run_tool(){
	opt -load $BUILD_DIR/lib/lib${TOOL_NAME}.so \
	-${TOOL_NAME} ${TEST_FILE} > /dev/null
}

create_ir(){
	cd ${TEST_DIR}
	if [ $(contains "${BENCHMARKS[@]}" "$TEST_NAME") == "y" ] ;then
		make b${TEST_NAME}
	else
		make all -j$(nproc)
	fi
	cd ${BASE_DIR}
}

clean_ir(){
	cd ${TEST_DIR}
	if [ $(contains "${BENCHMARKS[@]}" "$TEST_NAME") == "y" ] ;then
		make c${TEST_NAME}
	else
		make clean
	fi
	cd ${BASE_DIR}
}

run_checker(){
	create_ir
	run_make
	run_tool
}

run_tests(){
	TEMP_FILE=temp.txt
	TEST_TYPES=("prop" "unit")
	for TT in "${TEST_TYPES[@]}"; do
		for CHK in "${CHECKERS[@]}"; do
			MODE=${CHK}
			TEST_NAME="${TT}${CHK}"
			echo ${MODE} ${TEST_NAME}
			run &> ${TEMP_FILE}
			cat ${TEMP_FILE} | grep bugs
		done
	done
	rm ${TEMP_FILE}
}

run(){
	init_test
	if [ $(contains "${ALL_CHECKERS[@]}" "$MODE") == "y" ] ;then
		run_checker
	elif [ "$MODE" == "make" ] ;then
		run_make
	elif [ "$MODE" == "build" ] ;then
		run_fullbuild
	elif [ "$MODE" == "ir" ] ;then
		create_ir
	elif [ "$MODE" == "remir" ] ;then
		clean_ir
	elif [ "$MODE" == "test" ] ;then
		run_tests
	elif [ "$MODE" == "check" ] ;then
		run_tests
	else
		echo "pair, dur, log, make, build, ir, remir, test, check"
	fi
}

#commands----------------------------------------------------
run