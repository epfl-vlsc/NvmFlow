# useful stuff

#$(LL_DIR)/%.ll: $(SRC_DIR)/%.bc
	#llvm-dis -o $@ $<