;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: sar16.asm 1.1 1997/12/10 16:48:10 jon Exp $
;; $Locker: $
;;
;; Out of line call for gauranteed SAR
;;
	.386p
	.model flat,c
ifndef __WATCOMC__
	.code
_sar16	proc a:dword
	mov	eax,a
	sar	eax,16
	ret
_sar16	endp
endif

	end

