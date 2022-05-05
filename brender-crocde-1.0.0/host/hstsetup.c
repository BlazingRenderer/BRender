/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: hstsetup.c 2.14 1997/08/13 10:42:24 JOHNG Exp $
 * $Locker: $
 *
 * Setup for host extender (registers DLLs for drivers)
 */
#include "brddi.h"

#include "host.h"
#include "hostimg.h"
#include "host_ip.h"

void BR_PUBLIC_ENTRY HostBegin(void)
{
	BrImageAdd(&Image_BRHOST1);
}
 
void BR_PUBLIC_ENTRY HostEnd(void)
{
	BrImageRemove(&Image_BRHOST1);
}

static host_info hostInfo = {
	sizeof(host_info),
#if defined(__WIN_32__)
	"Microsoft WIN32",
	0,
#endif
	BRT_INTEL,
	BRT_INTEL_386,
};

br_error BR_RESIDENT_ENTRY HostInfo(host_info *buffer, br_size_t buffersize)
{
	br_int_32 features;
	br_value use_mmx;
	br_value use_cmov;

	if(buffersize < sizeof(host_info)) {
		BrMemCpy(buffer, &hostInfo, buffersize);
		return BRE_OVERFLOW;
	}

	BrMemCpy(buffer, &hostInfo, sizeof(host_info));

	/*
	 * Set processor type field
	 */
	CPUInfo(&buffer->processor_type, &features);
	buffer->capabilities |= features;

	/*
	 * If the USE_MMX flag is present, and is false, disable the MMX
	 * capability
	 */
    if (BrSystemConfigQuery(BRT_BRENDER_USE_MMX_B, &use_mmx) == BRE_OK && !use_mmx.b)
		buffer->capabilities &= ~HOST_CAPS_MMX;

	/*
	 * If the USE_CMOV flag is present, and is false, disable the CMOV
	 * capability
	 */
// The following line has been commented out to avoid problems with dodgy cyrix processors.
    if (BrSystemConfigQuery(BRT_BRENDER_USE_CMOV_B, &use_cmov) == BRE_OK && !use_cmov.b)
		buffer->capabilities &= ~HOST_CAPS_CMOV;

	
	return BRE_OK;
}

