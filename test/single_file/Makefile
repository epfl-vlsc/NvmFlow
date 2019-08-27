

BUILD_DIR="../../dfbuild"
SRC_DIR := .
BC_DIR := .
LL_DIR := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
BC_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BC_DIR)/%.bc,$(SRC_FILES))
LL_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(LL_DIR)/%.ll,$(SRC_FILES))
CHECKER_FLAGS := -fno-discard-value-names -ggdb -femit-all-decls -fstandalone-debug
INC_FLAGS := -I./..
LLVM_FLAGS := -emit-llvm ${CHECKER_FLAGS} -c
CONS_FLAGS := -load $(BUILD_DIR)/lib/libcons.so -cons
INITOPT := -O0 -Xclang -disable-O0-optnone

#opt -debug-pass=Structure

$(BC_DIR)/%.bc: $(SRC_DIR)/%.cpp
	clang++ $(INITOPT) $(LLVM_FLAGS) $(INC_FLAGS) -o $@ $<
	opt -mem2reg $(CONS_FLAGS) -o $@ $@ > /dev/null 2>&1
	#opt $(CONS_FLAGS) -o $@ $@ > /dev/null 2>&1
	
$(LL_DIR)/%.ll: $(SRC_DIR)/%.bc
	llvm-dis -o $@ $<

bc: $(BC_FILES)

ll: $(LL_FILES)

all: bc ll

clean:
	rm *.bc *.ll