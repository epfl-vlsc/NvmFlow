#pragma once

#define MACRO_PARAM_STRINGIFY(x) #x
#define MACRO_TOSTRING(x) MACRO_PARAM_STRINGIFY(x)

#define sentinel(CHECKER)                                                     \
  __attribute((annotate("pair-" MACRO_TOSTRING(CHECKER))))

// pointer checker
#define dur_field __attribute((annotate("DurableField")))

#define skip_fnc __attribute((annotate("SkipCode")))
#define nvm_fnc __attribute((annotate("NvmCode")))
#define tx_fnc __attribute((annotate("TxCode")))

#define no_inline __attribute__((optnone))

extern void no_inline vfence();
extern void no_inline pfence();
extern void no_inline pm_flush(void const* p);
extern void no_inline pm_flushfence(void const* p);

extern void no_inline tx_log(void* ptr);
extern void no_inline tx_begin();
extern void no_inline tx_end();

extern int cond();
extern void mutate(void* ptr);
extern void* pm_malloc(unsigned long size);
