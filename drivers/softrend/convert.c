/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: convert.c 1.2 1997/12/17 13:09:16 jon Exp $
 * $Locker: $
 *
 * Primitive renderer filters that will convert vertex components to desired destination format
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"


#define FCONV	(float)
#define XCONV	BrFloatToFixed
#define ICONV	(br_int_32)

void ConvertVertex(brp_vertex *dest, brp_vertex *src)
{
	int c;
	br_uint_32 m;

	m = rend.block->convert_mask_f;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1)
				dest[0].comp_f[c] = FCONV(src->comp[c]);

	m = rend.block->convert_mask_x;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1)
				dest[0].comp_x[c] = XCONV(src->comp[c]);

	m = rend.block->convert_mask_i;
	if (m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1)
				dest[0].comp_i[c] = ICONV(src->comp[c]);

	if(m = 0x0000ffff ^ (rend.block->convert_mask_i | rend.block->convert_mask_x | rend.block->convert_mask_f))
		for(c = 0; m; c++, m >>=1)
			if(m & 1)
				dest[0].comp[c] = src->comp[c];

}

void BR_ASM_CALL RenderConvert1(struct brp_block *block,
	brp_vertex *v0)
{
	int c;
	br_uint_32 m;
	brp_vertex outv[1];

	m = rend.block->convert_mask_f;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1)
				outv[0].comp_f[c] = FCONV(v0->comp[c]);

	m = rend.block->convert_mask_x;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1)
				outv[0].comp_x[c] = XCONV(v0->comp[c]);

	m = rend.block->convert_mask_i;
	if (m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1)
				outv[0].comp_i[c] = ICONV(v0->comp[c]);

	if(m=0x0000ffff^(rend.block->convert_mask_i|rend.block->convert_mask_x|rend.block->convert_mask_f))
		for(c = 0; m; c++, m >>=1)
			if(m & 1)
                                outv[0].comp_i[c] = v0->comp_i[c];

	block->chain->render(block->chain, outv);
}

void BR_ASM_CALL RenderConvert2(struct brp_block *block,
	brp_vertex *v0, brp_vertex *v1)
{
	int c;
	br_uint_32 m;
	brp_vertex outv[2];

	m = rend.block->convert_mask_f;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
				outv[0].comp_f[c] = FCONV(v0->comp[c]);
				outv[1].comp_f[c] = FCONV(v1->comp[c]);
			}

	m = rend.block->convert_mask_x;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
				outv[0].comp_x[c] = XCONV(v0->comp[c]);
				outv[1].comp_x[c] = XCONV(v1->comp[c]);
			}

	m = rend.block->convert_mask_i;
	if (m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
				outv[0].comp_i[c] = ICONV(v0->comp[c]);
				outv[1].comp_i[c] = ICONV(v1->comp[c]);
			}

	if(m=0x0000ffff^(rend.block->convert_mask_i|rend.block->convert_mask_x|rend.block->convert_mask_f))
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
                                outv[0].comp_i[c] = v0->comp_i[c];
                                outv[1].comp_i[c] = v1->comp_i[c];
			}

	block->chain->render(block->chain, outv+0, outv+1);
}

void BR_ASM_CALL RenderConvert3(struct brp_block *block,
	brp_vertex *v0, brp_vertex *v1,brp_vertex *v2)
{
	int c;
	br_uint_32 m;
	brp_vertex outv[3];

	m = rend.block->convert_mask_f;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
				outv[0].comp_f[c] = FCONV(v0->comp[c]);
				outv[1].comp_f[c] = FCONV(v1->comp[c]);
				outv[2].comp_f[c] = FCONV(v2->comp[c]);
			}

	m = rend.block->convert_mask_x;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
				outv[0].comp_x[c] = XCONV(v0->comp[c]);
				outv[1].comp_x[c] = XCONV(v1->comp[c]);
				outv[2].comp_x[c] = XCONV(v2->comp[c]);
			}

	m = rend.block->convert_mask_i;
	if (m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
				outv[0].comp_i[c] = ICONV(v0->comp[c]);
				outv[1].comp_i[c] = ICONV(v1->comp[c]);
				outv[2].comp_i[c] = ICONV(v2->comp[c]);
			}

	if(m=0x0000ffff^(rend.block->convert_mask_i|rend.block->convert_mask_x|rend.block->convert_mask_f))
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
                                outv[0].comp_i[c] = v0->comp_i[c];
                                outv[1].comp_i[c] = v1->comp_i[c];
                                outv[2].comp_i[c] = v2->comp_i[c];
			}

	block->chain->render(block->chain, outv+0, outv+1, outv+2);
}

void BR_ASM_CALL RenderConvert4(struct brp_block *block,
	brp_vertex *v0, brp_vertex *v1,brp_vertex *v2,brp_vertex *v3)
{
	int c;
	br_uint_32 m;
	brp_vertex outv[4];

	m = rend.block->convert_mask_f;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
				outv[0].comp_f[c] = FCONV(v0->comp[c]);
				outv[1].comp_f[c] = FCONV(v1->comp[c]);
				outv[2].comp_f[c] = FCONV(v2->comp[c]);
				outv[3].comp_f[c] = FCONV(v3->comp[c]);
			}

	m = rend.block->convert_mask_x;
	if(m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
				outv[0].comp_x[c] = XCONV(v0->comp[c]);
				outv[1].comp_x[c] = XCONV(v1->comp[c]);
				outv[2].comp_x[c] = XCONV(v2->comp[c]);
				outv[3].comp_x[c] = XCONV(v3->comp[c]);
			}

	m = rend.block->convert_mask_i;
	if (m)
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
				outv[0].comp_i[c] = ICONV(v0->comp[c]);
				outv[1].comp_i[c] = ICONV(v1->comp[c]);
				outv[2].comp_i[c] = ICONV(v2->comp[c]);
				outv[3].comp_i[c] = ICONV(v3->comp[c]);
			}

	if(m=0x0000ffff^(rend.block->convert_mask_i|rend.block->convert_mask_x|rend.block->convert_mask_f))
		for(c = 0; m; c++, m >>=1)
			if(m & 1) {
                                outv[0].comp_i[c] = v0->comp_i[c];
                                outv[1].comp_i[c] = v1->comp_i[c];
                                outv[2].comp_i[c] = v2->comp_i[c];
                                outv[3].comp_i[c] = v3->comp_i[c];
			}

	block->chain->render(block->chain, outv+0, outv+1, outv+2, outv+3);
}

