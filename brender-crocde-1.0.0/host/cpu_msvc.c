#include "cpu.h"

void cpu_cpuid(cpuid_regs *cpu_regs, br_uint_32 info_type)
{
	__asm 
	{
		mov    esi, cpu_regs
		mov    eax, info_type
		xor    ecx, ecx  
		cpuid  
		mov    dword ptr [esi +  0], eax
		mov    dword ptr [esi +  4], ebx  
		mov    dword ptr [esi +  8], ecx  
		mov    dword ptr [esi + 12], edx  
	}
}

br_uint_32 cpu_getflags()
{
	br_uint_32 f;
	__asm
	{
		pushfd
		pop f
	}

	return f;
}

void cpu_setflags(br_uint_32 flags)
{
	__asm
	{
		push flags
		popfd
	}
}
