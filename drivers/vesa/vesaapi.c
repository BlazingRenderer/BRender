/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: vesaapi.c 1.1 1997/12/10 16:54:28 jon Exp $
 * $Locker: $
 *
 * Device methods
 */
#include <stddef.h>
#include <string.h>
#include <brender.h>
#include <dpmi.h>
#include <go32.h>

#include "vesaapi.h"

#define SCRATCH_SIZE 1024

#define __tb_sel     (__tb >> 4)
#define __tb_off     (__tb & 0x0f)

#define MAX_MODES    (sizeof(vip->reserved) / sizeof(br_uint_16))

BR_STATIC_ASSERT(sizeof(struct vesa_info) == sizeof(struct vesa_info_rm), "sizeof(struct vesa_info) != sizeof(struct vesa_info_rm)");

static size_t RealStrLen(uint16_t seg, uint16_t off)
{
    size_t len = 0;

    for(unsigned char ch;;) {
        dosmemget((seg * (1 << 4)) + off + len, 1, &ch);

        if(ch == '\0')
            break;

        len++;
    }

    return len;
}

static char *RealResStrDup(void *vres, br_uint_16 seg, br_uint_16 off)
{
    size_t len = RealStrLen(seg, off);
    char  *buf = BrResAllocate(vres, (len + 1) * sizeof(char), BR_MEMORY_STRING);
    dosmemget((seg * (1 << 4)) + off, len, buf);
    buf[len] = '\0';
    return buf;
}

static void *read_modes(void *vres, br_uint_16 seg, br_uint_16 off, br_size_t *count)
{
    br_size_t    n;
    br_uint_16  *modes;
    br_uintptr_t addr = (seg * (1 << 4)) + off;

    for(n = 0;; ++n) {
        br_uint_16 mode;
        _dosmemgetw(addr + (n * sizeof(br_uint_16)), 1, &mode);
        if(mode == 0xFFFF) {
            break;
        }
    }

    modes = BrResAllocate(vres, sizeof(br_uint_16) * (n + 1), BR_MEMORY_DRIVER);
    dosmemget((seg * (1 << 4)) + off, sizeof(br_uint_16) * n, modes);
    modes[n] = 0xFFFF;

    if(count != NULL)
        *count = n;

    return modes;
}

br_error VESAInfo(struct vesa_info *vip)
{
    struct vesa_info_rm *vip_rm;
    __dpmi_regs          r;

    if(__tb_size < SCRATCH_SIZE)
        return BRE_FAIL;

    /*
     * Call VBE status interrupt
     */
    /* VBE 2.0 applications should preset this field with the ASCII characters 'VBE2' */
    BrMemCpy(vip->vbe_signature, "VBE2", 4);
    _dosmemputl(vip, 1, __tb);
    r = (__dpmi_regs){
        .x = {.ax = 0x4f00, .di = __tb_off, .es = __tb_sel},
    };
    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    /*
     * Copy vesa info. block into user's block
     */
    dosmemget(__tb, sizeof(struct vesa_info), vip);

    /* Upon return from VBE Function 00h, this field should always be set to 'VESA' by the VBE implementation. */
    if(BrMemCmp(vip->vbe_signature, "VESA", 4) != 0)
        return BRE_FAIL;

    vip_rm = (struct vesa_info_rm *)vip;

    /*
     * Grab modes
     */
    vip->videomode_ptr = read_modes(vip, vip_rm->videomode_seg, vip_rm->videomode_off, NULL);

    /*
     * Grab OEM strings.
     */
    vip->oem_string_ptr = (br_uint_8 *)RealResStrDup(vip, vip_rm->oem_string_seg, vip_rm->oem_string_off);

    if(vip_rm->vbe_version >= 0x200) {
        if(vip->oem_vendor_name_ptr != NULL)
            vip->oem_vendor_name_ptr = (br_uint_8 *)RealResStrDup(vip, vip_rm->oem_vendor_name_seg, vip_rm->oem_vendor_name_off);

        if(vip->oem_product_name_ptr != NULL)
            vip->oem_product_name_ptr = (br_uint_8 *)RealResStrDup(vip, vip_rm->oem_product_name_seg, vip_rm->oem_product_name_off);

        if(vip->oem_product_rev_ptr != NULL)
            vip->oem_product_rev_ptr = (br_uint_8 *)RealResStrDup(vip, vip_rm->oem_product_rev_seg, vip_rm->oem_product_rev_off);
    } else {
        vip->oem_vendor_name_ptr  = NULL;
        vip->oem_product_name_ptr = NULL;
        vip->oem_product_rev_ptr  = NULL;
    }

    if(vip->oem_vendor_name_ptr == NULL)
        vip->oem_vendor_name_ptr = (br_uint_8 *)BrResStrDup(vip, "<empty>");

    if(vip->oem_product_name_ptr == NULL)
        vip->oem_product_name_ptr = (br_uint_8 *)BrResStrDup(vip, "<empty>");

    if(vip->oem_product_rev_ptr == NULL)
        vip->oem_product_rev_ptr = (br_uint_8 *)BrResStrDup(vip, "<empty>");

    return BRE_OK;
}

br_error VESAModeInfo(struct vesa_modeinfo *vmip, br_uint_32 mode)
{
    __dpmi_regs r = {0};

    /*
     * Call VBE mode information
     */
    r.d.eax = 0x4f01;
    r.x.cx  = mode;
    r.x.es  = __tb_sel;
    r.x.di  = __tb_off;
    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    /*
     * Copy mode info. block into user's block
     */
    dosmemget(__tb, sizeof(struct vesa_modeinfo), vmip);

    return BRE_OK;
}

br_error VESAModeSet(br_uint_32 mode)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f02;
    r.x.bx        = mode;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    BrLogError("XX", "SET MODE %04x", mode);
    return BRE_OK;
}

br_error VESAModeGet(br_uint_16 *mode)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f03;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    *mode = r.x.bx;

    return BRE_OK;
}

br_error VESAScanlineBytesSet(br_uint_32 width, br_uint_16 *bytes, br_uint_16 *pixels, br_uint_16 *scanlines)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f06;
    r.h.bl        = 2;
    r.x.cx        = width;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    *bytes     = r.x.bx;
    *pixels    = r.x.cx;
    *scanlines = r.x.dx;

    return BRE_OK;
}

br_error VESAScanlinePixelsSet(br_uint_32 width, br_uint_16 *bytes, br_uint_16 *pixels, br_uint_16 *scanlines)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f06;
    r.h.bl        = 0;
    r.x.cx        = width;
    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    *bytes     = r.x.bx;
    *pixels    = r.x.cx;
    *scanlines = r.x.dx;

    return BRE_OK;
}

br_error VESAScanlineLengthGet(br_uint_16 *bytes, br_uint_16 *pixels, br_uint_16 *scanlines)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f06;
    r.h.bl        = 1;
    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    *bytes     = r.x.bx;
    *pixels    = r.x.cx;
    *scanlines = r.x.dx;

    return BRE_OK;
}

br_error VESADisplayStartSet(br_uint_32 x, br_uint_32 y)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f07;
    r.h.bl        = 0;
    r.x.cx        = x;
    r.x.dx        = y;
    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    return BRE_OK;
}

br_error VESADisplayStartGet(br_uint_32 *px, br_uint_32 *py)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f07;
    r.h.bl        = 1;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    *px = r.x.cx;
    *py = r.x.dx;

    return BRE_OK;
}

br_error VESAWindowSet(br_uint_32 window, br_uint_32 position)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f05;
    r.h.bh        = 0;
    r.h.bl        = window;
    r.x.dx        = position;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    return BRE_OK;
}

br_error VESAWindowGet(br_uint_32 window, br_uint_32 *position)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f05;
    r.h.bh        = 1;
    r.h.bl        = window;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    *position = r.x.dx;

    return BRE_OK;
}

br_error VESAPaletteFormatSet(br_uint_32 format)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f08;
    r.h.bl        = 0;
    r.h.bh        = format;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    return BRE_OK;
}

br_error VESAPaletteFormatGet(br_uint_32 *formatp)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f08;
    r.h.bl        = 1;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    *formatp = r.h.bh;

    return BRE_OK;
}

br_error VESAPaletteSet(br_int_32 *values, br_int_32 first, br_int_32 count, br_boolean blank)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f09;
    r.h.bl        = 0;
    r.x.cx        = count;
    r.x.dx        = first;
    r.x.es        = __tb_sel;
    r.x.di        = __tb_off;

    dosmemput(values, count * sizeof(*values), __tb);

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    return BRE_OK;
}

br_error VESAPaletteGet(br_int_32 *values, br_int_32 first, br_int_32 count)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f09;
    r.h.bl        = 1;
    r.x.cx        = count;
    r.x.dx        = first;
    r.x.es        = __tb_sel;
    r.x.di        = __tb_off;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    dosmemget(__tb, count * sizeof(*values), values);

    return BRE_OK;
}

br_error VESAProtectedModeInterface(br_uint_16 *poffset, br_uint_16 *pseg, br_uint_16 *psize)
{
    __dpmi_regs r = {0};
    r.d.eax       = 0x4f0a;
    r.h.bl        = 0;

    __dpmi_int(0x10, &r);

    if(r.h.al != 0x4f)
        return BRE_UNSUPPORTED;

    if(r.h.ah != 0x00)
        return BRE_FAIL;

    *poffset = r.x.di;
    *pseg    = r.x.es;
    *psize   = r.x.cx;

    return BRE_OK;
}
