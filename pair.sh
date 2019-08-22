MODE=$1 #build, pair, dur, log
TEST_NAME=$2 #under single file

#directories
BASE_DIR=$(dirname $(realpath "$0"))
BUILD_DIR="${BASE_DIR}/dfbuild"
TEST_DIR="${BASE_DIR}/test"
SINGLE_FILE_REPO=${TEST_DIR}/pair

LLVM_BASE_DIR=~/llvm_compiler8
COMPILER_DIR=${LLVM_BASE_DIR}/bin
LLVM_DIR=${LLVM_BASE_DIR}/build/lib/cmake/llvm/

#variables
TOOL_NAME=${MODE}

run_make() {
    cd ${BUILD_DIR}
    make -j$(nproc)
    cd ${BASE_DIR}
}

create_ir() {
    cd ${SINGLE_FILE_REPO}
    make all -j$(nproc)
    cd ${BASE_DIR}
}

#--debug-pass=Structure
run_tool() {
    opt \
        -load $BUILD_DIR/lib/lib${TOOL_NAME}.so -${TOOL_NAME} \
        ${SINGLE_FILE_REPO}/${TEST_NAME}.bc >/dev/null
}

#commands----------------------------------------------------
create_ir
run_make
run_tool
#commands----------------------------------------------------
