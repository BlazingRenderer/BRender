/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: devclut.c 1.1 1997/12/10 16:45:06 jon Exp $
 * $Locker: $
 *
 * CLUT methods
 */
#include <stddef.h>
#include <string.h>
#include <windows.h>
#include <ddraw.h>
#include <conio.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

BR_RCS_ID("$Id: devclut.c 1.1 1997/12/10 16:45:06 jon Exp $");

/*
 * Default dispatch table for device_clut (defined at end of file)
 */
static const struct br_device_clut_dispatch deviceClutDispatch;

/*
 * Query template
 */
#define F(f)	offsetof(struct br_device_clut, f)

static struct br_tv_template_entry deviceClutTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	0,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
};
#undef F


/*
 * Create a new device CLUT
 */
br_device_clut * DeviceClutDirectDrawAllocate(br_device *dev, br_device_pixelmap *devpm, const char *identifier)
{
	int i, half_nstatic;
	br_size_t size;
	br_device_clut *self;
	HDC dc;
	LOGPALETTE *logpalette;

	UASSERT(dev != NULL);
	UASSERT(devpm != NULL);

	/*
	 * Allocate device CLUT structure
	 */
	self = BrResAllocate( dev->res, sizeof(*self), BR_MEMORY_OBJECT );

	/*
	 * Fill members
	 */
	self->dispatch = &deviceClutDispatch;
	if(identifier)
	 	self->identifier = identifier;
	self->devpm = devpm;
	self->device = dev ;

	/*
	 * Create a logical palette
	 */
	dc = GetDC(NULL);
	self->nstatic = GetDeviceCaps(dc, NUMCOLORS);
	self->syspal_size = GetDeviceCaps(dc, SIZEPALETTE);

	/*
	 * Use all the colours if we are running fullscreen
	 */
	if (self->devpm != NULL && self->devpm->fullscreen)
		self->nstatic = 0;	// 2 before I discovered ALLOW256

	size = sizeof(LOGPALETTE) + self->syspal_size * sizeof(PALETTEENTRY);
	logpalette = (LOGPALETTE *)BrResAllocate(self, size, BR_MEMORY_OBJECT_DATA);
	self->logpalette = logpalette;

	logpalette->palVersion = 0x300;
	logpalette->palNumEntries = self->syspal_size;

	GetSystemPaletteEntries(dc, 0, self->syspal_size, &logpalette->palPalEntry[0]);
	ReleaseDC(NULL, dc);

	half_nstatic = self->nstatic / 2;

	for(i = 0; i < half_nstatic ; i++)
		logpalette->palPalEntry[i].peFlags = 0;
	for(; i < self->syspal_size - half_nstatic; i++)
		logpalette->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
	for(; i < self->syspal_size; i++)
		logpalette->palPalEntry[i].peFlags = 0;

	self->palette = CreatePalette(logpalette);
	devpm->hpal = self->palette;

	/*
	 * Attach CLUT to device
	 */
	ObjectContainerAddFront(dev, (br_object *)self);

	return self;
}

/*
 * br_device_clut_dd::free
 */
static void BR_CMETHOD_DECL(br_device_clut_dd, free)(br_object *_self)
{
	br_device_clut *self = (br_device_clut*)_self;

	/*
	 * Detach CLUT from device
	 */
	ObjectContainerRemove( self->device, (br_object *)self);

	/*
	 * Release the logical palette
	 */
	DeleteObject(self->palette);

	/*
	 * Free resources
	 */
	BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_device_clut_dd, type)(br_object *self)
{
	return BRT_DEVICE_CLUT;
}

static br_boolean BR_CMETHOD_DECL(br_device_clut_dd, isType)(br_object *self, br_token t)
{
	return (t == BRT_DEVICE_CLUT) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_device_clut_dd, space)(br_object *self)
{
	return sizeof(br_device_clut);
}

static struct br_tv_template * BR_CMETHOD_DECL(br_device_clut_dd, queryTemplate)(br_object *_self)
{
    br_device_clut *self = (br_device_clut*)_self;
    if(self->device->templates.deviceClutTemplate== NULL)
        self->device->templates.deviceClutTemplate= BrTVTemplateAllocate(self->device,
            deviceClutTemplateEntries,
            BR_ASIZE(deviceClutTemplateEntries));

    return self->device->templates.deviceClutTemplate;
}

/*
 * br_device_clut_dd::entrySet
 */
static br_error BR_CMETHOD_DECL(br_device_clut_dd, entrySet)\
	(br_device_clut *self, br_int_32 index, br_colour entry)
{
	int half_nstatic;
	PALETTEENTRY palette_entry;

	/*
	 * Check index does not reference a static colour
	 */
	half_nstatic = self->nstatic / 2;

	if(index < half_nstatic)
		return BRE_FAIL;
	if(index >= (self->syspal_size - half_nstatic))
		return BRE_FAIL;

	/*
	 * Fill palette entry
	 */
	palette_entry.peRed = (BYTE)BR_RED(entry);
	palette_entry.peGreen = (BYTE)BR_GRN(entry);
	palette_entry.peBlue = (BYTE)BR_BLU(entry);
	palette_entry.peFlags = PC_NOCOLLAPSE;

	/*
	 * Set the single palette entry
	 */
	SetPaletteEntries(self->palette, index, 1, &palette_entry);
	IDirectDrawPalette_SetEntries(self->devpm->palette, 0, index, 1, &palette_entry);
	IDirectDrawSurface_SetPalette(self->devpm->surface, self->devpm->palette);

	return BRE_OK;
}

/*
 * br_device_clut_dd::entrySetMany
 */
static br_error BR_CMETHOD_DECL(br_device_clut_dd, entrySetMany)\
	(br_device_clut *self, br_int_32 index, br_int_32 count, br_colour *entries)
{
	int i, delta, half_nstatic;
	br_colour entry;
	PALETTEENTRY *palette_entries;

	/*
	 * Clamp index and count to non-static colours
	 */
	half_nstatic = self->nstatic / 2;

	delta = half_nstatic - index;
	if(delta > 0) {
		count -= delta;
		entries += delta;
		index = half_nstatic;
	}

	delta = index + count;
	delta -= self->syspal_size - half_nstatic;
	if(delta > 0)
		count -= delta;

	/*
	 * Check there are still entries to set
	 */
	if(count <= 0)
		return BRE_FAIL;

	/*
	 * Allocate temporary workspace
	 */
	palette_entries = (PALETTEENTRY *)BrMemAllocate(count * sizeof(PALETTEENTRY), BR_MEMORY_SCRATCH);

	/*
	 * Fill palette entry structures
	 */
	for(i = 0; i < count; i++) {
		entry = entries[i];
		palette_entries[i].peRed = (BYTE)BR_RED(entry);
		palette_entries[i].peGreen = (BYTE)BR_GRN(entry);
		palette_entries[i].peBlue = (BYTE)BR_BLU(entry);
		palette_entries[i].peFlags = PC_NOCOLLAPSE;
	}

	/*
	 * Set the palette entries
	 */
	SetPaletteEntries(self->palette, index, count, palette_entries);
	IDirectDrawPalette_SetEntries(self->devpm->palette, 0, index, count, palette_entries);
	IDirectDrawSurface_SetPalette(self->devpm->surface, self->devpm->palette);

	/*
	 * Free workspace
	 */
	BrMemFree(palette_entries);

	return BRE_OK;
}

/*
 * br_device_clut_dd::entryQuery
 */
static br_error BR_CMETHOD_DECL(br_device_clut_dd, entryQuery)\
	(br_device_clut *self, br_colour *entry, br_int_32 index)
{
	return BRE_FAIL;
}

/**
 ** br_device_clut_dd::entryQueryMany
 **/
static br_error BR_CMETHOD_DECL(br_device_clut_dd, entryQueryMany)\
	(br_device_clut *self, br_colour *entries, br_int_32 index, br_int_32 count)
{
	return BRE_FAIL;
}

/*
 * Default dispatch table for device CLUT
 */
static const struct br_device_clut_dispatch deviceClutDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_clut_dd, free),
    ._identifier = BR_CMETHOD_REF(br_object_dd, identifier),
    ._type       = BR_CMETHOD_REF(br_device_clut_dd, type),
    ._isType     = BR_CMETHOD_REF(br_device_clut_dd, isType),
    ._device     = BR_CMETHOD_REF(br_object_dd, device),
    ._space      = BR_CMETHOD_REF(br_device_clut_dd, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_clut_dd, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._entrySet       = BR_CMETHOD_REF(br_device_clut_dd, entrySet),
    ._entryQuery     = BR_CMETHOD_REF(br_device_clut_dd, entryQuery),
    ._entrySetMany   = BR_CMETHOD_REF(br_device_clut_dd, entrySetMany),
    ._entryQueryMany = BR_CMETHOD_REF(br_device_clut_dd, entryQueryMany),
};
