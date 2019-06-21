#pragma once

#include <stdio.h>

#define MACRO_PARAM_STRINGIFY(x) #x
#define MACRO_TOSTRING(x) MACRO_PARAM_STRINGIFY(x)

#define sentinelp(CHECKER) __attribute((annotate("pair-" MACRO_TOSTRING(CHECKER))))
#define sentinel __attribute((annotate("sent")))

//pointer checker
#define p_ptr __attribute((annotate("FlushedPtr")))

#define skip_fnc __attribute((annotate("SkipCode")))
#define nvm_fnc __attribute((annotate("NvmCode")))

#define log_field __attribute((annotate("LogField")))
#define analyze_logging __attribute((annotate("LogCode")))

void vfence() { }
void pfence() {  }
void clflushopt(void const* p) { }
void clflush(void const* p) { }

void log(void* ptr) {}
void tx_begin() {}
void tx_end() {}
