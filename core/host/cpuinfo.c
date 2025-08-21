#include "host.h"

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#include <cpuid.h>

static int x86_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    return __get_cpuid(leaf, eax, ebx, ecx, edx);
}

#define CPUINFO_PROC CPUInfo_x86

#elif defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#include <intrin.h>

static int x86_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    int CPUInfo[4];
    __cpuid(CPUInfo, leaf);
    memcpy(eax, CPUInfo + 0);
    memcpy(ebx, CPUInfo + 1);
    memcpy(ecx, CPUInfo + 2);
    memcpy(edx, CPUInfo + 3);
    return 1;
}

#define CPUINFO_PROC CPUInfo_x86

#else

static int x86_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    return 0;
}
#endif

static void CPUInfo_Default(host_info *info)
{
    /*
     * Keep existing behaviour. This should be changed in the future.
     */
    info->processor_family = BRT_INTEL;
    info->processor_type   = BRT_INTEL_PENTIUM_PRO;
    info->capabilities |= HOST_CAPS_FPU | HOST_CAPS_MMX | HOST_CAPS_CMOV;
}

static void CPUInfo_x86(host_info *info)
{
    unsigned int eax, ebx, ecx, edx;
    char         vendor[13];

    info->processor_family = BRT_INTEL;
    info->processor_type   = BRT_INTEL_PENTIUM_PRO;
    if(x86_cpuid(0, &eax, &ebx, &ecx, &edx) == 0) {
        return;
    }

    BrMemCpy(vendor + 0, &ebx, sizeof(ebx));
    BrMemCpy(vendor + 4, &edx, sizeof(edx));
    BrMemCpy(vendor + 8, &ecx, sizeof(ecx));
    vendor[12] = '\0';

    BrStrNCpy(info->vendor_string, vendor, sizeof(info->vendor_string));
    info->vendor_string[BR_ASIZE(info->vendor_string) - 1] = '\0';

    // clang-format off
    struct {
        unsigned int bit;
        br_uint_32   cap;
    } edx_bits[] = {
        {.bit =  0, .cap = HOST_CAPS_FPU },
        {.bit = 15, .cap = HOST_CAPS_CMOV},
        {.bit = 23, .cap = HOST_CAPS_MMX },
        {.bit = 25, .cap = HOST_CAPS_SSE },
        {.bit = 26, .cap = HOST_CAPS_SSE2},
    }, ecx_bits[] = {
        {.bit =  0, .cap = HOST_CAPS_SSE3  },
        {.bit =  9, .cap = HOST_CAPS_SSSE3 },
        {.bit = 19, .cap = HOST_CAPS_SSE4_1},
        {.bit = 20, .cap = HOST_CAPS_SSE4_2},
        {.bit = 23, .cap = HOST_CAPS_POPCNT},
        {.bit = 28, .cap = HOST_CAPS_AVX   },
    };
    // clang-format on

    if(x86_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        unsigned int family     = (eax >> 8) & 0xF;
        unsigned int ext_family = (eax >> 20) & 0xFF;

        if(family == 0xF)
            family += ext_family;

        if(family == 4) {
            info->processor_type = BRT_INTEL_486;
        } else if(family == 5) {
            info->processor_type = BRT_INTEL_PENTIUM;
        } else if(family >= 6) {
            info->processor_type = BRT_INTEL_PENTIUM_PRO;
        }

        for(br_size_t i = 0; i < BR_ASIZE(ecx_bits); ++i) {
            if(ecx & (1 << ecx_bits[i].bit))
                info->capabilities |= ecx_bits[i].cap;
        }

        for(br_size_t i = 0; i < BR_ASIZE(edx_bits); ++i) {
            if(edx & (1 << edx_bits[i].bit))
                info->capabilities |= edx_bits[i].cap;
        }
    }
}

#if !defined(CPUINFO_PROC)
#define CPUINFO_PROC CPUInfo_Default
#endif

void CPUInfo(host_info *info)
{
    CPUINFO_PROC(info);
}
