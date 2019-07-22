#pragma once

#include <stdio.h>

#define MACRO_PARAM_STRINGIFY(x) #x
#define MACRO_TOSTRING(x) MACRO_PARAM_STRINGIFY(x)

#define sentinel(CHECKER)                                                     \
  __attribute((annotate("pair-" MACRO_TOSTRING(CHECKER))))

// pointer checker
#define p_ptr __attribute((annotate("FlushedPtr")))

#define skip_fnc __attribute((annotate("SkipCode")))
#define nvm_fnc __attribute((annotate("NvmCode")))
#define log_fnc __attribute((annotate("LogCode")))

#define log_field __attribute((annotate("LogField")))

#define no_inline __attribute__((noinline))

void no_inline vfence(){}
void no_inline pfence(){}
void no_inline clflushopt(void const* p){}
void no_inline clflush(void const* p){}

void no_inline log(void* ptr){}
void no_inline tx_begin(){}
void no_inline tx_end(){}
