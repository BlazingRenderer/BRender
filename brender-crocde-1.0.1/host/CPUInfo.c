#include "brender.h"
#include "host.h"
#include "cpu.h"

static br_token cpu_types[7] =
{
	BRT_INTEL_386,			// 0
	BRT_INTEL_386,			// 1
	BRT_INTEL_386,			// 2
	BRT_INTEL_386,			// 3
	BRT_INTEL_486,			// 4
	BRT_INTEL_PENTIUM,		// 5
	BRT_INTEL_PENTIUM_PRO	// 6
};

/**
 * Get the CPU Information.
 * @param cpu_type A pointer to a br_token that will receive the type of the CPU.<br>
 * May be one of {BRT_INTEL_386, BRT_INTEL_486, BRT_INTEL_PENTIUM, BRT_INTEL_PENTIUM_PRO}
 * @param features A pointer to a br_uint_32 that will receive the features supported by
 * the CPU.<br>
 * Will be a bitwise combination of {HOST_CAPS_FPU, HOST_CAPS_MMX, HOST_CAPS_CMOV}
 */
void CPUInfo(br_token *cpu_type, br_uint_32 *features)
{
	br_uint_32 flags, oldFlags;
	cpuid_regs regs = {0, 0, 0, 0};

	/* Default to no features on a 80386 */
	*features = 0;
	*cpu_type = BRT_INTEL_386;

	flags = oldFlags = cpu_getflags();

	
	/* Check if we're on an 80386 (Flip AC bit) */
	flags ^= 0x40000;

	cpu_setflags(flags);
	flags = cpu_getflags();
	
	/* Can't toggle AC bit, processor=80386 */
	if(!(flags ^ oldFlags))
		return;

	/* Restore the flags */
	cpu_setflags(oldFlags);
	

	/* Now check if we're on a 80486 (Flip ID bit) */
	*cpu_type = BRT_INTEL_486;
	flags = oldFlags ^ 0x200000;
	
	cpu_setflags(flags);
	flags = cpu_getflags();
	
	/* Can't toggle ID bit, processor=80486 */
	if(!(flags ^ oldFlags))
		return;

	/* Make sure EAX=1 (Processor Info and Feature Bits) is supported */
	cpu_cpuid(&regs, 0);
	if(regs.eax < 1)
		return;
	
	/* Get family/model/stepping/features */
	cpu_cpuid(&regs, 1);
	
	/* Isolate family */
	regs.eax = (regs.eax >> 8) & 0xf;
	
	/* Treat any future CPUs as Pentium PRO's */
	if(regs.eax > 6)
		regs.eax = 6;
	
	/* Convert the family to a token. */
	*cpu_type = cpu_types[regs.eax];
	
	if(regs.edx & 0x1)
		*features |= HOST_CAPS_FPU;
	if(regs.edx & 0x800000)
		*features |= HOST_CAPS_MMX;
	if(regs.edx & 0x8000)
		*features |= HOST_CAPS_CMOV;
	
	/* Cyrix is dead, not putting in the check. */
	/*
		mov		eax,00000000
		cpuid                  ; cpuid w/eax=0, gets the VendorID
		cmp		ebx,069727943h    ; "Cyri"
		jnz		NotCyrix
		cmp		edx,0736e4978h    ; "xins"
		jnz		NotCyrix
		cmp		ecx,064616574h    ; "tead"
		jnz		NotCyrix
		mov		eax,[esi]
		and		eax,NOT HOST_CAPS_CMOV   ; clear CMOV bit
		mov		[esi],eax	
	*/
}