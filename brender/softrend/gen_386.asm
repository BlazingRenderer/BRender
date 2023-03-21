;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: gen_386.asm 1.2 1998/07/20 14:45:11 jon Exp $
;; $Locker: $
;;
;; Downcoded general geometry support
;;
	.586p
	.model flat,c

	include	drv.inc
	include 586_macs.inc

if ADD_RCS_ID
	.data
	db '$Id: gen_386.asm 1.2 1998/07/20 14:45:11 jon Exp $',0
	align	16
endif

	.code

	if BASED_FIXED	; Only provide these when driver is fixed point

	; Need to be converted to use separate face equations and
	; vertex indices
	;
	if 0

	include vertex.inc


; Premultiply viewport into model->screen matrix 
;
; scache.model_to_viewport[0..3][0] = scache.model_to_screen[0.[0..3][0] * scales[X]
; scache.model_to_viewport[0..3][1] = scache.model_to_screen[0..3][1] *  scales[Y]
; scache.model_to_viewport[0..3][2] = scache.model_to_screen[0..3][2] * -1
;
; Keep a 64 bit result for row 3 - the translations and perspective
; that will not be multiplied (incoming points are x,y,z,1)
;
; Return !0 if there was an overflow
;
ModelToViewportUpdate_A proc uses ebx esi edi

	; Loop for rows 0-2
	;
				mov	ecx,3
				xor	esi,esi

row_loop:
		; X
				mov	eax, scache.model_to_screen.m[esi+0]
				imul	scache.scale_x

				mov	ebx,edx
				and	ebx,0ffff8000h
				je	no_x_overflow
				cmp	ebx,0ffff8000h
				jne	overflowed

no_x_overflow:	shr	eax,16
				shl	edx,16
				or	eax,edx

				mov	scache.model_to_viewport.m[esi+0],eax

		; Y
				mov	eax,scache.model_to_screen.m[esi+4]
				imul	scache.scale_y

				mov	ebx,edx
				and	ebx,0ffff8000h
				je	no_y_overflow
				cmp	ebx,0ffff8000h
				jne	overflowed

no_y_overflow:	shr	eax,16
				shl	edx,16
				or	eax,edx

				mov	scache.model_to_viewport.m[esi+4],eax


	; Y
				mov	eax,scache.model_to_screen.m[esi+8]
				imul	scache.scale_z

				mov	ebx,edx
				and	ebx,0ffff8000h
				je	no_z_overflow
				cmp	ebx,0ffff8000h
				jne	overflowed

no_z_overflow:	shr	eax,16
				shl	edx,16
				or	eax,edx

				mov	scache.model_to_viewport.m[esi+8],eax


		; W
				mov	eax,scache.model_to_screen.m[esi+12]
				mov	scache.model_to_viewport.m[esi+12],eax

				add	esi,16
				dec	ecx
				jne	row_loop

	; row 3
	;
		; X
				mov	eax, scache.model_to_screen.m[48+0]
				imul	scache.scale_x
				mov	scache.model_to_viewport.m[48+0],eax
				mov	scache.model_to_viewport_hi[0],edx

		; Y
				mov	eax,scache.model_to_screen.m[48+4]
				imul	scache.scale_y
				mov	scache.model_to_viewport.m[48+4],eax
				mov	scache.model_to_viewport_hi[4],edx

		; Z
				mov	eax,scache.model_to_screen.m[48+8]
				imul	scache.scale_z
				mov	scache.model_to_viewport.m[48+8],eax
				mov	scache.model_to_viewport_hi[8],edx

		; W
				mov	eax,scache.model_to_screen.m[48+12]
				mov	scache.model_to_viewport.m[48+12],eax

				xor	eax,eax
				ret
overflowed:
				mov	eax,-1
				ret

ModelToViewportUpdate_A endp

	endif

	endif	; BASED_FIXED
end

