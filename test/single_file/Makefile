SRC_DIR := .
BC_DIR := .
LL_DIR := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
BC_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BC_DIR)/%.bc,$(SRC_FILES))
LL_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(LL_DIR)/%.ll,$(SRC_FILES))
CPPFLAGS := -ggdb -c -emit-llvm -femit-all-decls -fstandalone-debug

$(BC_DIR)/%.bc: $(SRC_DIR)/%.cpp
	clang++ $(CPPFLAGS) -o $@ $<

$(LL_DIR)/%.ll: $(SRC_DIR)/%.bc
	llvm-dis -o $@ $<

all: $(BC_FILES) $(LL_FILES)

clean:
	rm *.bc *.ll