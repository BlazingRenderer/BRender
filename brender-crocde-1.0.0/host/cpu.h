#ifndef __CPU_H__
#define __CPU_H_

#include "compiler.h"

typedef struct {
	br_uint_32 eax;
	br_uint_32 ebx;
	br_uint_32 ecx;
	br_uint_32 edx;
} cpuid_regs;

#ifdef __cplusplus
extern "C" {
#endif

void		cpu_cpuid(cpuid_regs *cpu_regs, br_uint_32 info_type);
br_uint_32	cpu_getflags(void);
void		cpu_setflags(br_uint_32 flags);

#ifdef __cplusplus
}
#endif

#endif /* __CPU_H__ */