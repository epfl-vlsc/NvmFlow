# useful stuff

#$(LL_DIR)/%.ll: $(SRC_DIR)/%.bc
	#llvm-dis -o $@ $<

#
$(LL_DIR)/%.ll: $(SRC_DIR)/%.cpp
	clang++ -S $(INITOPT) $(CPPFLAGS) $(INCFLAGS) -o $@ $<
	opt -S $(CONSFLAGS) -o $@ $@ > /dev/null 2>&1
	clang++ -S $(FINOPT) $(CPPFLAGS) -o $@ $@