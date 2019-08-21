#pragma once

#include <stdio.h>

#define MACRO_PARAM_STRINGIFY(x) #x
#define MACRO_TOSTRING(x) MACRO_PARAM_STRINGIFY(x)

#define sentinel(CHECKER)                                                     \
  __attribute((annotate("pair-" MACRO_TOSTRING(CHECKER))))

// pointer checker
#define dur_field __attribute((annotate("DurableField")))

#define skip_fnc __attribute((annotate("SkipCode")))
#define nvm_fnc __attribute((annotate("NvmCode")))
#define log_fnc __attribute((annotate("LogCode")))

#define log_field __attribute((annotate("LogField")))

#define no_inline __attribute__((noinline))

extern void no_inline vfence();
extern void no_inline pfence();
extern void no_inline clflushopt(void const* p);
extern void no_inline clflush(void const* p);

extern void no_inline tx_log(void* ptr);
extern void no_inline tx_begin();
extern void no_inline tx_end();
