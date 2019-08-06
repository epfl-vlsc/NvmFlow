SRC_DIR := .
LL_DIR := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.bc)
LL_FILES := $(patsubst $(SRC_DIR)/%.bc,$(LL_DIR)/%.ll,$(SRC_FILES))

$(LL_DIR)/%.ll: $(SRC_DIR)/%.bc
	llvm-dis -o $@ $<

ll: $(LL_FILES)

all: ll

clean:
	rm *.ll