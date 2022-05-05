;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: memloops.asm 2.5 1996/12/03 20:02:10 jon Exp $
;; $Locker: $
;;
;; Inner loops for operations on in-memory pixelmaps
;;
	.386p
	.model flat,c

	.code

; br_uint_16 BR_ASM_CALL _GetSysQual(void);
;
; Return a value for the qualifier argument that renders into system memory
;
_GetSysQual	proc
		mov	ax,ds
		ret
_GetSysQual	endp

	end

