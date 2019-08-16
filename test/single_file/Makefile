

BUILD_DIR="../../dfbuild"
SRC_DIR := .
BC_DIR := .
LL_DIR := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
BC_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BC_DIR)/%.bc,$(SRC_FILES))
LL_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(LL_DIR)/%.ll,$(SRC_FILES))
CHECKER_FLAGS := -fno-discard-value-names -ggdb -femit-all-decls -fstandalone-debug
INCFLAGS := -I./..
CPPFLAGS := -emit-llvm ${CHECKER_FLAGS} -c
CONSFLAGS := -load $(BUILD_DIR)/lib/libcons.so -cons
INITOPT := -O1 -mllvm -disable-llvm-optzns
FINOPT := -O3

$(BC_DIR)/%.bc: $(SRC_DIR)/%.cpp
	clang++ $(INITOPT) $(CPPFLAGS) $(INCFLAGS) -o $@ $<
	opt $(CONSFLAGS) -o $@ $@ > /dev/null 2>&1
	clang++ $(FINOPT) $(CPPFLAGS) -o $@ $@
	
#$(LL_DIR)/%.ll: $(SRC_DIR)/%.bc
	#llvm-dis -o $@ $<
$(LL_DIR)/%.ll: $(SRC_DIR)/%.cpp
	clang++ -S $(INITOPT) $(CPPFLAGS) $(INCFLAGS) -o $@ $<
	opt -S $(CONSFLAGS) -o $@ $@ > /dev/null 2>&1
	clang++ -S $(FINOPT) $(CPPFLAGS) -o $@ $@

bc: $(BC_FILES)

ll: $(LL_FILES)

all: ll bc

clean:
	rm *.bc *.ll