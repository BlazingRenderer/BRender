;; Copyright (c) 1991,1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: fntp7x9.asm 1.1 1997/12/10 16:41:23 jon Exp $
;; $Locker: $
;;
	.386p
	.model flat

	.data
		db '$Id: fntp7x9.asm 1.1 1997/12/10 16:41:23 jon Exp $',0
		align 4
	.data
	public __FontProp7x9

__FontProp7x9 label dword
	dd	1			; flags - PROPORTIONAL
	dw	7			; glyph_x
	dw	9			; glyph_y
	dw	8			; spacing_x
	dw	10			; spacing_y
	dd	_FontP7x9_Width		; width table
	dd	_FontP7x9_Encoding	; encoding table
	dd	_FontP7x9_Glyphs	; Start of glyph data

GLYPH	macro name
	dw	glyph_&name&-_FontP7x9_Glyphs
	endm

GWIDTH	macro name
	db	width_&name&
	endm

	; Encoding table
	;

_FontP7x9_Encoding label word
	GLYPH space 	; 00 NUL
	GLYPH space 	; 01 
	GLYPH space 	; 02 
	GLYPH space 	; 03 
	GLYPH space 	; 04 
	GLYPH space 	; 05 
	GLYPH space 	; 06 
	GLYPH bullet 	; 07 
	GLYPH space 	; 08 
	GLYPH space 	; 09
	GLYPH space 	; 0A
	GLYPH space 	; 0B 
	GLYPH space 	; 0C 
	GLYPH space 	; 0D
	GLYPH space 	; 0E 
	GLYPH space 	; 0F 
	GLYPH space	; 10 
	GLYPH space	; 11 
	GLYPH space 	; 12 
	GLYPH space 	; 13 
	GLYPH space 	; 14 
	GLYPH space 	; 15 
	GLYPH space 	; 16 
	GLYPH space 	; 17 
	GLYPH uparrow 	; 18 
	GLYPH downarrow	; 19 
	GLYPH rightarrow ; 1A 
	GLYPH leftarrow	; 1B 
	GLYPH space 	; 1C 
	GLYPH space 	; 1D 
	GLYPH space 	; 1E 
	GLYPH space 	; 1F 
	GLYPH space 	; 20 SPC
	GLYPH exclam 	; 21 !
	GLYPH dquote 	; 22 "
	GLYPH hash  	; 23 #
	GLYPH dollar 	; 24 $
	GLYPH percent 	; 25 %
	GLYPH ampersand ; 26 &
	GLYPH quote 	; 27 '
	GLYPH lparen 	; 28 (
	GLYPH rparen 	; 29 )
	GLYPH asterisk 	; 2A *
	GLYPH plus 	; 2B +
	GLYPH comma 	; 2C ,
	GLYPH minus 	; 2D -
	GLYPH period 	; 2E .
	GLYPH slash 	; 2F /
	GLYPH 0 	; 30 0
	GLYPH 1 	; 31 1
	GLYPH 2 	; 32 2
	GLYPH 3 	; 33 3
	GLYPH 4 	; 34 4
	GLYPH 5 	; 35 5
	GLYPH 6 	; 36 6
	GLYPH 7 	; 37 7
	GLYPH 8 	; 38 8
	GLYPH 9 	; 39 9
	GLYPH colon 	; 3A :
	GLYPH semicolon ; 3B ;
	GLYPH langle 	; 3C <
	GLYPH equal 	; 3D =
	GLYPH rangle 	; 3E >
	GLYPH query 	; 3F ?
	GLYPH comat 	; 40 @
	GLYPH A 	; 41 A
	GLYPH B 	; 42 B
	GLYPH C 	; 43 C
	GLYPH D 	; 44 D
	GLYPH E 	; 45 E
	GLYPH F 	; 46 F
	GLYPH G 	; 47 G
	GLYPH H 	; 48 H
	GLYPH I 	; 49 I
	GLYPH J 	; 4A J
	GLYPH K 	; 4B K
	GLYPH L 	; 4C L
	GLYPH M 	; 4D M
	GLYPH N 	; 4E N
	GLYPH O 	; 4F O
	GLYPH P 	; 50 P
	GLYPH Q 	; 51 Q
	GLYPH R 	; 52 R
	GLYPH S 	; 53 S
	GLYPH T 	; 54 T
	GLYPH U 	; 55 U
	GLYPH V 	; 56 V
	GLYPH W 	; 57 W
	GLYPH X 	; 58 X
	GLYPH Y 	; 59 Y
	GLYPH Z 	; 5A Z
	GLYPH lsquare 	; 5B [
	GLYPH bslash 	; 5C \
	GLYPH rsquare 	; 5D ]
	GLYPH caret 	; 5E ^
	GLYPH underbar 	; 5F _
	GLYPH bquote 	; 60 `
	GLYPH a 	; 61 a
	GLYPH b 	; 62 b
	GLYPH c 	; 63 c
	GLYPH d 	; 64 d
	GLYPH e 	; 65 e
	GLYPH f 	; 66 f
	GLYPH g 	; 67 g
	GLYPH h 	; 68 h
	GLYPH i 	; 69 i
	GLYPH j 	; 6A j
	GLYPH k 	; 6B k
	GLYPH l 	; 6C l
	GLYPH m 	; 6D m
	GLYPH n 	; 6E n
	GLYPH o 	; 6F o
	GLYPH p 	; 70 p
	GLYPH q 	; 71 q
	GLYPH r 	; 72 r
	GLYPH s 	; 73 s
	GLYPH t 	; 74 t
	GLYPH u 	; 75 u
	GLYPH v 	; 76 v
	GLYPH w 	; 77 w
	GLYPH x 	; 78 x
	GLYPH y 	; 79 y
	GLYPH z 	; 7A z
	GLYPH lbrace 	; 7B {
	GLYPH vbar 	; 7C |
	GLYPH rbrace 	; 7D }
	GLYPH tilde 	; 7E ~
	GLYPH space 	; 7F 
	GLYPH space 	; 80 �
	GLYPH space 	; 81 �
	GLYPH space 	; 82 �
	GLYPH space	; 83 �
	GLYPH space	; 84 �
	GLYPH space	; 85 �
	GLYPH space 	; 86 �
	GLYPH space 	; 87 �
	GLYPH space 	; 88 �
	GLYPH space 	; 89 �
	GLYPH space 	; 8A �
	GLYPH space 	; 8B �
	GLYPH space 	; 8C �
	GLYPH space 	; 8D �
	GLYPH space 	; 8E �
	GLYPH space 	; 8F �
	GLYPH space 	; 90 �
	GLYPH space 	; 91 �
	GLYPH space 	; 92 �
	GLYPH space 	; 93 �
	GLYPH space 	; 94 �
	GLYPH space 	; 95 �
	GLYPH space 	; 96 �
	GLYPH space 	; 97 �
	GLYPH space 	; 98 �
	GLYPH space 	; 99 �
	GLYPH space 	; 9A �
	GLYPH space 	; 9B �
	GLYPH space 	; 9C �
	GLYPH space 	; 9D �
	GLYPH space 	; 9E �
	GLYPH space 	; 9F �
	GLYPH space 	; A0 �
	GLYPH space 	; A1 �
	GLYPH space 	; A2 �
	GLYPH space 	; A3 �
	GLYPH space 	; A4 �
	GLYPH space 	; A5 �
	GLYPH space 	; A6 �
	GLYPH space 	; A7 �
	GLYPH space 	; A8 �
	GLYPH space 	; A9 �
	GLYPH space 	; AA �
	GLYPH space 	; AB �
	GLYPH space 	; AC �
	GLYPH space 	; AD �
	GLYPH space 	; AE �
	GLYPH space 	; AF �
	GLYPH space 	; B0 �
	GLYPH space 	; B1 �
	GLYPH space 	; B2 �
	GLYPH cursor 	; B3 �
	GLYPH space 	; B4 �
	GLYPH space 	; B5 �
	GLYPH space 	; B6 �
	GLYPH space 	; B7 �
	GLYPH space 	; B8 �
	GLYPH space 	; B9 �
	GLYPH space 	; BA �
	GLYPH space 	; BB �
	GLYPH space 	; BC �
	GLYPH space 	; BD �
	GLYPH space 	; BE �
	GLYPH space 	; BF �
	GLYPH space 	; C0 �
	GLYPH space 	; C1 �
	GLYPH space 	; C2 �
	GLYPH space 	; C3 �
	GLYPH space 	; C4 �
	GLYPH space 	; C5 �
	GLYPH space 	; C6 �
	GLYPH space 	; C7 �
	GLYPH space 	; C8 �
	GLYPH space 	; C9 �
	GLYPH space 	; CA �
	GLYPH space 	; CB �
	GLYPH space 	; CC �
	GLYPH space 	; CD �
	GLYPH space 	; CE �
	GLYPH space 	; CF �
	GLYPH space 	; D0 �
	GLYPH space 	; D1 �
	GLYPH space 	; D2 �
	GLYPH space 	; D3 �
	GLYPH space 	; D4 �
	GLYPH space 	; D5 �
	GLYPH space 	; D6 �
	GLYPH space 	; D7 �
	GLYPH space 	; D8 �
	GLYPH space 	; D9 �
	GLYPH space 	; DA �
	GLYPH solidbox 	; DB �
	GLYPH space 	; DC �
	GLYPH space 	; DD �
	GLYPH space 	; DE �
	GLYPH space 	; DF �
	GLYPH space 	; E0 �
	GLYPH space 	; E1 �
	GLYPH space 	; E2 �
	GLYPH space 	; E3 �
	GLYPH space 	; E4 �
	GLYPH space 	; E5 �
	GLYPH space 	; E6 �
	GLYPH space 	; E7 �
	GLYPH space 	; E8 �
	GLYPH space 	; E9 �
	GLYPH space 	; EA �
	GLYPH space 	; EB �
	GLYPH space 	; EC �
	GLYPH space 	; ED �
	GLYPH space 	; EE �
	GLYPH space 	; EF �
	GLYPH space 	; F0 �
	GLYPH space 	; F1 �
	GLYPH space 	; F2 �
	GLYPH space 	; F3 �
	GLYPH space 	; F4 �
	GLYPH space 	; F5 �
	GLYPH space 	; F6 �
	GLYPH space 	; F7 �
	GLYPH space 	; F8 �
	GLYPH space 	; F9 �
	GLYPH space 	; FA �
	GLYPH tick 	; FB �
	GLYPH space 	; FC �
	GLYPH space 	; FD �
	GLYPH space 	; FE �
	GLYPH space 	; FF �


_FontP7x9_Width label byte
	GWIDTH space 	; 00 NUL
	GWIDTH space 	; 01 
	GWIDTH space 	; 02 
	GWIDTH space 	; 03 
	GWIDTH space 	; 04 
	GWIDTH space 	; 05 
	GWIDTH space 	; 06 
	GWIDTH bullet 	; 07 
	GWIDTH space 	; 08 
	GWIDTH space 	; 09
	GWIDTH space 	; 0A
	GWIDTH space 	; 0B 
	GWIDTH space 	; 0C 
	GWIDTH space 	; 0D
	GWIDTH space 	; 0E 
	GWIDTH space 	; 0F 
	GWIDTH space 	; 10 
	GWIDTH space 	; 11 
	GWIDTH space 	; 12 
	GWIDTH space 	; 13 
	GWIDTH space 	; 14 
	GWIDTH space 	; 15 
	GWIDTH space 	; 16 
	GWIDTH space 	; 17 
	GWIDTH uparrow	; 18 
	GWIDTH downarrow; 19 
	GWIDTH rightarrow 	; 1A 
	GWIDTH leftarrow 	; 1B 
	GWIDTH space 	; 1C 
	GWIDTH space 	; 1D 
	GWIDTH space 	; 1E 
	GWIDTH space 	; 1F 
	GWIDTH space 	; 20 SPC
	GWIDTH exclam 	; 21 !
	GWIDTH dquote 	; 22 "
	GWIDTH hash  	; 23 #
	GWIDTH dollar 	; 24 $
	GWIDTH percent 	; 25 %
	GWIDTH ampersand ; 26 &
	GWIDTH quote 	; 27 '
	GWIDTH lparen 	; 28 (
	GWIDTH rparen 	; 29 )
	GWIDTH asterisk 	; 2A *
	GWIDTH plus 	; 2B +
	GWIDTH comma 	; 2C ,
	GWIDTH minus 	; 2D -
	GWIDTH period 	; 2E .
	GWIDTH slash 	; 2F /
	GWIDTH 0 	; 30 0
	GWIDTH 1 	; 31 1
	GWIDTH 2 	; 32 2
	GWIDTH 3 	; 33 3
	GWIDTH 4 	; 34 4
	GWIDTH 5 	; 35 5
	GWIDTH 6 	; 36 6
	GWIDTH 7 	; 37 7
	GWIDTH 8 	; 38 8
	GWIDTH 9 	; 39 9
	GWIDTH colon 	; 3A :
	GWIDTH semicolon ; 3B ;
	GWIDTH langle 	; 3C <
	GWIDTH equal 	; 3D =
	GWIDTH rangle 	; 3E >
	GWIDTH query 	; 3F ?
	GWIDTH comat 	; 40 @
	GWIDTH A 	; 41 A
	GWIDTH B 	; 42 B
	GWIDTH C 	; 43 C
	GWIDTH D 	; 44 D
	GWIDTH E 	; 45 E
	GWIDTH F 	; 46 F
	GWIDTH G 	; 47 G
	GWIDTH H 	; 48 H
	GWIDTH I 	; 49 I
	GWIDTH J 	; 4A J
	GWIDTH K 	; 4B K
	GWIDTH L 	; 4C L
	GWIDTH M 	; 4D M
	GWIDTH N 	; 4E N
	GWIDTH O 	; 4F O
	GWIDTH P 	; 50 P
	GWIDTH Q 	; 51 Q
	GWIDTH R 	; 52 R
	GWIDTH S 	; 53 S
	GWIDTH T 	; 54 T
	GWIDTH U 	; 55 U
	GWIDTH V 	; 56 V
	GWIDTH W 	; 57 W
	GWIDTH X 	; 58 X
	GWIDTH Y 	; 59 Y
	GWIDTH Z 	; 5A Z
	GWIDTH lsquare 	; 5B [
	GWIDTH bslash 	; 5C \
	GWIDTH rsquare 	; 5D ]
	GWIDTH caret 	; 5E ^
	GWIDTH underbar 	; 5F _
	GWIDTH bquote 	; 60 `
	GWIDTH a 	; 61 a
	GWIDTH b 	; 62 b
	GWIDTH c 	; 63 c
	GWIDTH d 	; 64 d
	GWIDTH e 	; 65 e
	GWIDTH f 	; 66 f
	GWIDTH g 	; 67 g
	GWIDTH h 	; 68 h
	GWIDTH i 	; 69 i
	GWIDTH j 	; 6A j
	GWIDTH k 	; 6B k
	GWIDTH l 	; 6C l
	GWIDTH m 	; 6D m
	GWIDTH n 	; 6E n
	GWIDTH o 	; 6F o
	GWIDTH p 	; 70 p
	GWIDTH q 	; 71 q
	GWIDTH r 	; 72 r
	GWIDTH s 	; 73 s
	GWIDTH t 	; 74 t
	GWIDTH u 	; 75 u
	GWIDTH v 	; 76 v
	GWIDTH w 	; 77 w
	GWIDTH x 	; 78 x
	GWIDTH y 	; 79 y
	GWIDTH z 	; 7A z
	GWIDTH lbrace 	; 7B {
	GWIDTH vbar 	; 7C |
	GWIDTH rbrace 	; 7D }
	GWIDTH tilde 	; 7E ~
	GWIDTH space 	; 7F 
	GWIDTH space 	; 80 �
	GWIDTH space 	; 81 �
	GWIDTH space 	; 82 �
	GWIDTH space	; 83 �
	GWIDTH space	; 84 �
	GWIDTH space	; 85 �
	GWIDTH space 	; 86 �
	GWIDTH space 	; 87 �
	GWIDTH space 	; 88 �
	GWIDTH space 	; 89 �
	GWIDTH space 	; 8A �
	GWIDTH space 	; 8B �
	GWIDTH space 	; 8C �
	GWIDTH space 	; 8D �
	GWIDTH space 	; 8E �
	GWIDTH space 	; 8F �
	GWIDTH space 	; 90 �
	GWIDTH space 	; 91 �
	GWIDTH space 	; 92 �
	GWIDTH space 	; 93 �
	GWIDTH space 	; 94 �
	GWIDTH space 	; 95 �
	GWIDTH space 	; 96 �
	GWIDTH space 	; 97 �
	GWIDTH space 	; 98 �
	GWIDTH space 	; 99 �
	GWIDTH space 	; 9A �
	GWIDTH space 	; 9B �
	GWIDTH space 	; 9C �
	GWIDTH space 	; 9D �
	GWIDTH space 	; 9E �
	GWIDTH space 	; 9F �
	GWIDTH space 	; A0 �
	GWIDTH space 	; A1 �
	GWIDTH space 	; A2 �
	GWIDTH space 	; A3 �
	GWIDTH space 	; A4 �
	GWIDTH space 	; A5 �
	GWIDTH space 	; A6 �
	GWIDTH space 	; A7 �
	GWIDTH space 	; A8 �
	GWIDTH space 	; A9 �
	GWIDTH space 	; AA �
	GWIDTH space 	; AB �
	GWIDTH space 	; AC �
	GWIDTH space 	; AD �
	GWIDTH space 	; AE �
	GWIDTH space 	; AF �
	GWIDTH space 	; B0 �
	GWIDTH space 	; B1 �
	GWIDTH space 	; B2 �
	GWIDTH cursor 	; B3 �
	GWIDTH space 	; B4 �
	GWIDTH space 	; B5 �
	GWIDTH space 	; B6 �
	GWIDTH space 	; B7 �
	GWIDTH space 	; B8 �
	GWIDTH space 	; B9 �
	GWIDTH space 	; BA �
	GWIDTH space 	; BB �
	GWIDTH space 	; BC �
	GWIDTH space 	; BD �
	GWIDTH space 	; BE �
	GWIDTH space 	; BF �
	GWIDTH space 	; C0 �
	GWIDTH space 	; C1 �
	GWIDTH space 	; C2 �
	GWIDTH space 	; C3 �
	GWIDTH space 	; C4 �
	GWIDTH space 	; C5 �
	GWIDTH space 	; C6 �
	GWIDTH space 	; C7 �
	GWIDTH space 	; C8 �
	GWIDTH space 	; C9 �
	GWIDTH space 	; CA �
	GWIDTH space 	; CB �
	GWIDTH space 	; CC �
	GWIDTH space 	; CD �
	GWIDTH space 	; CE �
	GWIDTH space 	; CF �
	GWIDTH space 	; D0 �
	GWIDTH space 	; D1 �
	GWIDTH space 	; D2 �
	GWIDTH space 	; D3 �
	GWIDTH space 	; D4 �
	GWIDTH space 	; D5 �
	GWIDTH space 	; D6 �
	GWIDTH space 	; D7 �
	GWIDTH space 	; D8 �
	GWIDTH space 	; D9 �
	GWIDTH space 	; DA �
	GWIDTH solidbox	; DB �
	GWIDTH space 	; DC �
	GWIDTH space 	; DD �
	GWIDTH space 	; DE �
	GWIDTH space 	; DF �
	GWIDTH space 	; E0 �
	GWIDTH space 	; E1 �
	GWIDTH space 	; E2 �
	GWIDTH space 	; E3 �
	GWIDTH space 	; E4 �
	GWIDTH space 	; E5 �
	GWIDTH space 	; E6 �
	GWIDTH space 	; E7 �
	GWIDTH space 	; E8 �
	GWIDTH space 	; E9 �
	GWIDTH space 	; EA �
	GWIDTH space 	; EB �
	GWIDTH space 	; EC �
	GWIDTH space 	; ED �
	GWIDTH space 	; EE �
	GWIDTH space 	; EF �
	GWIDTH space 	; F0 �
	GWIDTH space 	; F1 �
	GWIDTH space 	; F2 �
	GWIDTH space 	; F3 �
	GWIDTH space 	; F4 �
	GWIDTH space 	; F5 �
	GWIDTH space 	; F6 �
	GWIDTH space 	; F7 �
	GWIDTH space 	; F8 �
	GWIDTH space 	; F9 �
	GWIDTH space 	; FA �
	GWIDTH tick 	; FB �
	GWIDTH space 	; FC �
	GWIDTH space 	; FD �
	GWIDTH space 	; FE �
	GWIDTH space 	; FF �
		
.radix 2

_FontP7x9_Glyphs label byte

; Proportional 7x9 Font
;
width_space	equ 03h
glyph_space	db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000

width_exclam	equ 01h
glyph_exclam	db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 00000000
		db 10000000
		db 00000000
		db 00000000

width_dquote	equ 03h
glyph_dquote	db 10100000
		db 10100000
		db 10100000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000

width_hash	equ 05h
glyph_hash	db 01010000
		db 01010000
		db 11111000
		db 01010000
		db 11111000
		db 01010000
		db 01010000
		db 00000000
		db 00000000

width_dollar	equ 05h
glyph_dollar	db 00100000
		db 01110000
		db 10000000
		db 01110000
		db 00001000
		db 01110000
		db 00100000
		db 00000000
		db 00000000

width_percent	equ 07h
glyph_percent	db 01000010
		db 10100100
		db 01001000
		db 00010000
		db 00100100
		db 01001010
		db 10000100
		db 00000000
		db 00000000

width_ampersand	equ 05h
glyph_ampersand	db 00100000
		db 01010000
		db 00100000
		db 01100000
		db 10011000
		db 10010000
		db 01101000
		db 00000000
		db 00000000

width_quote	equ 02h
glyph_quote	db 01000000
		db 10000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000

width_lparen	equ 03h
glyph_lparen	db 00100000
		db 01000000
		db 10000000
		db 10000000
		db 10000000
		db 01000000
		db 00100000
		db 00000000
		db 00000000

width_rparen	equ 03h
glyph_rparen	db 10000000
		db 01000000
		db 00100000
		db 00100000
		db 00100000
		db 01000000
		db 10000000
		db 00000000
		db 00000000

width_asterisk	equ 05h
glyph_asterisk	db 01010000
		db 00100000
		db 11111000
		db 00100000
		db 01010000
		db 00000000
		db 00000000
		db 00000000
		db 00000000

width_plus	equ 05h
glyph_plus	db 00000000
		db 00100000
		db 00100000
		db 11111000
		db 00100000
		db 00100000
		db 00000000
		db 00000000
		db 00000000

width_comma	equ 02h
glyph_comma	db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 01000000
		db 10000000
		db 00000000

width_minus	equ 05h
glyph_minus	db 00000000
		db 00000000
		db 00000000
		db 11111000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000

width_period	equ 02h
glyph_period	db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 01000000
		db 00000000
		db 00000000

width_slash	equ 07h
glyph_slash	db 00000010
		db 00000100
		db 00001000
		db 00010000
		db 00100000
		db 01000000
		db 10000000
		db 00000000
		db 00000000

width_0	equ 05h
glyph_0		db 01110000
		db 10001000
		db 10001000
		db 10001000
		db 10001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_1	equ 05h
glyph_1		db 00100000
		db 01100000
		db 00100000
		db 00100000
		db 00100000
		db 00100000
		db 01110000
		db 00000000
		db 00000000

width_2	equ 05h
glyph_2		db 01110000
		db 10001000
		db 00001000
		db 00010000
		db 00100000
		db 01000000
		db 11111000
		db 00000000
		db 00000000

width_3	equ 05h
glyph_3		db 01110000
		db 10001000
		db 00001000
		db 00110000
		db 00001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_4	equ 05h
glyph_4		db 00010000
		db 00110000
		db 01010000
		db 10010000
		db 11111000
		db 00010000
		db 00010000
		db 00000000
		db 00000000

width_5	equ 05h
glyph_5		db 11111000
		db 10000000
		db 11110000
		db 00001000
		db 00001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_6	equ 05h
glyph_6		db 00110000
		db 01000000
		db 10000000
		db 11110000
		db 10001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_7	equ 05h
glyph_7		db 11111000
		db 00001000
		db 00001000
		db 00010000
		db 00010000
		db 00100000
		db 00100000
		db 00000000
		db 00000000

width_8	equ 05h
glyph_8		db 01110000
		db 10001000
		db 10001000
		db 01110000
		db 10001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_9	equ 05h
glyph_9		db 01110000
		db 10001000
		db 10001000
		db 01111000
		db 00001000
		db 00010000
		db 01100000
		db 00000000
		db 00000000

width_colon	equ 02h
glyph_colon	db 00000000
		db 00000000
		db 00000000
		db 01000000
		db 00000000
		db 00000000
		db 01000000
		db 00000000
		db 00000000

width_semicolon	equ 02h
glyph_semicolon	db 00000000
		db 00000000
		db 00000000
		db 01000000
		db 00000000
		db 00000000
		db 01000000
		db 10000000
		db 00000000

width_langle	equ 04h
glyph_langle	db 00010000
		db 00100000
		db 01000000
		db 10000000
		db 01000000
		db 00100000
		db 00010000
		db 00000000
		db 00000000

width_equal	equ 05h
glyph_equal	db 00000000
		db 00000000
		db 11111000
		db 00000000
		db 00000000
		db 11111000
		db 00000000
		db 00000000
		db 00000000

width_rangle	equ 04h
glyph_rangle	db 10000000
		db 01000000
		db 00100000
		db 00010000
		db 00100000
		db 01000000
		db 10000000
		db 00000000
		db 00000000

width_query	equ 05h
glyph_query	db 01110000
		db 10001000
		db 00001000
		db 00010000
		db 00100000
		db 00000000
		db 00100000
		db 00000000
		db 00000000

width_comat	equ 06h
glyph_comat	db 01111000
		db 10000100
		db 10100100
		db 10100100
		db 10111000
		db 10000000
		db 01111000
		db 00000000
		db 00000000

width_A	equ 05h
glyph_A		db 00100000
		db 01010000
		db 10001000
		db 10001000
		db 11111000
		db 10001000
		db 10001000
		db 00000000
		db 00000000

width_B	equ 05h
glyph_B		db 11110000
		db 10001000
		db 10001000
		db 11110000
		db 10001000
		db 10001000
		db 11110000
		db 00000000
		db 00000000

width_C	equ 06h
glyph_C		db 00111000
		db 01000100
		db 10000000
		db 10000000
		db 10000000
		db 01000100
		db 00111000
		db 00000000
		db 00000000

width_D	equ 06h
glyph_D		db 11110000
		db 10001000
		db 10000100
		db 10000100
		db 10000100
		db 10001000
		db 11110000
		db 00000000
		db 00000000

width_E	equ 05h
glyph_E		db 11111000
		db 10000000
		db 10000000
		db 11100000
		db 10000000
		db 10000000
		db 11111000
		db 00000000
		db 00000000

width_F	equ 05h
glyph_F		db 11111000
		db 10000000
		db 10000000
		db 11100000
		db 10000000
		db 10000000
		db 10000000
		db 00000000
		db 00000000

width_G	equ 06h
glyph_G		db 00111000
		db 01000100
		db 10000000
		db 10000000
		db 10001100
		db 01000100
		db 00111100
		db 00000000
		db 00000000

width_H	equ 06h
glyph_H		db 10000100
		db 10000100
		db 10000100
		db 11111100
		db 10000100
		db 10000100
		db 10000100
		db 00000000
		db 00000000

width_I	equ 01h
glyph_I		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 00000000
		db 00000000

width_J	equ 05h
glyph_J		db 00001000
		db 00001000
		db 00001000
		db 00001000
		db 00001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_K	equ 05h
glyph_K		db 10001000
		db 10010000
		db 10100000
		db 11000000
		db 10100000
		db 10010000
		db 10001000
		db 00000000
		db 00000000

width_L	equ 05h
glyph_L		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 11111000
		db 00000000
		db 00000000

width_M	equ 07h
glyph_M		db 10000010
		db 11000110
		db 10101010
		db 10010010
		db 10000010
		db 10000010
		db 10000010
		db 00000000
		db 00000000

width_N	equ 07h
glyph_N		db 10000010
		db 11000010
		db 10100010
		db 10010010
		db 10001010
		db 10000110
		db 10000010
		db 00000000
		db 00000000

width_O	equ 07h
glyph_O		db 00111000
		db 01000100
		db 10000010
		db 10000010
		db 10000010
		db 01000100
		db 00111000
		db 00000000
		db 00000000

width_P	equ 05h
glyph_P		db 11110000
		db 10001000
		db 10001000
		db 11110000
		db 10000000
		db 10000000
		db 10000000
		db 00000000
		db 00000000

width_Q	equ 07h
glyph_Q		db 00111000
		db 01000100
		db 10000010
		db 10000010
		db 10000010
		db 01000100
		db 00111000
		db 00001110
		db 00000000

width_R	equ 05h
glyph_R		db 11110000
		db 10001000
		db 10001000
		db 11110000
		db 10100000
		db 10010000
		db 10001000
		db 00000000
		db 00000000

width_S	equ 05h
glyph_S		db 01110000
		db 10001000
		db 10000000
		db 01110000
		db 00001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_T	equ 07h
glyph_T		db 11111110
		db 00010000
		db 00010000
		db 00010000
		db 00010000
		db 00010000
		db 00010000
		db 00000000
		db 00000000

width_U	equ 05h
glyph_U		db 10001000
		db 10001000
		db 10001000
		db 10001000
		db 10001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_V	equ 05h
glyph_V		db 10001000
		db 10001000
		db 10001000
		db 10001000
		db 10001000
		db 01010000
		db 00100000
		db 00000000
		db 00000000

width_W	equ 09h
glyph_W		db 10001000,10000000
		db 10001000,10000000
		db 10001000,10000000
		db 10001000,10000000
		db 10001000,10000000
		db 01010101,00000000
		db 00100010,00000000
		db 00000000,00000000
		db 00000000,00000000

width_X	equ 07h
glyph_X		db 10000010
		db 01000100
		db 00101000
		db 00010000
		db 00101000
		db 01000100
		db 10000010
		db 00000000
		db 00000000

width_Y	equ 07h
glyph_Y		db 10000010
		db 01000100
		db 00101000
		db 00010000
		db 00010000
		db 00010000
		db 00010000
		db 00000000
		db 00000000

width_Z	equ 06h
glyph_Z		db 11111100
		db 00000100
		db 00001000
		db 00010000
		db 00100000
		db 01000000
		db 11111100
		db 00000000
		db 00000000

width_lsquare	equ 03h
glyph_lsquare	db 11100000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 11100000
		db 00000000
		db 00000000

width_bslash	equ 07h
glyph_bslash	db 10000000
		db 01000000
		db 00100000
		db 00010000
		db 00001000
		db 00000100
		db 00000010
		db 00000000
		db 00000000

width_rsquare	equ 03h
glyph_rsquare	db 11100000
		db 00100000
		db 00100000
		db 00100000
		db 00100000
		db 00100000
		db 11100000
		db 00000000
		db 00000000

width_caret	equ 05h
glyph_caret	db 00100000
		db 01010000
		db 10001000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000

width_underbar	equ 06h
glyph_underbar	db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 11111100

width_bquote	equ 02h
glyph_bquote	db 10000000
		db 01000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000

width_a	equ 05h
glyph_a		db 00000000
		db 00000000
		db 01110000
		db 00001000
		db 01111000
		db 10001000
		db 01111000
		db 00000000
		db 00000000

width_b	equ 05h
glyph_b		db 10000000
		db 10000000
		db 11110000
		db 10001000
		db 10001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_c	equ 04h
glyph_c		db 00000000
		db 00000000
		db 01110000
		db 10000000
		db 10000000
		db 10000000
		db 01110000
		db 00000000
		db 00000000

width_d	equ 05h
glyph_d		db 00001000
		db 00001000
		db 01111000
		db 10001000
		db 10001000
		db 10001000
		db 01111000
		db 00000000
		db 00000000

width_e	equ 05h
glyph_e		db 00000000
		db 00000000
		db 01110000
		db 10001000
		db 11111000
		db 10000000
		db 01110000
		db 00000000
		db 00000000

width_f	equ 04h
glyph_f		db 00110000
		db 01000000
		db 11100000
		db 01000000
		db 01000000
		db 01000000
		db 01000000
		db 00000000
		db 00000000

width_g	equ 05h
glyph_g		db 00000000
		db 00000000
		db 01110000
		db 10001000
		db 10001000
		db 10001000
		db 01111000
		db 00001000
		db 01110000

width_h	equ 05h
glyph_h		db 10000000
		db 10000000
		db 11110000
		db 10001000
		db 10001000
		db 10001000
		db 10001000
		db 00000000
		db 00000000

width_i	equ 01h
glyph_i		db 10000000
		db 00000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 00000000
		db 00000000

width_j	equ 02h
glyph_j		db 01000000
		db 00000000
		db 01000000
		db 01000000
		db 01000000
		db 01000000
		db 01000000
		db 01000000
		db 10000000

width_k	equ 04h
glyph_k		db 10000000
		db 10000000
		db 10010000
		db 10100000
		db 11000000
		db 10100000
		db 10010000
		db 00000000
		db 00000000

width_l	equ 01h
glyph_l		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 10000000
		db 00000000
		db 00000000

width_m	equ 07h
glyph_m		db 00000000
		db 00000000
		db 11101100
		db 10010010
		db 10010010
		db 10010010
		db 10010010
		db 00000000
		db 00000000

width_n	equ 05h
glyph_n		db 00000000
		db 00000000
		db 10110000
		db 11001000
		db 10001000
		db 10001000
		db 10001000
		db 00000000
		db 00000000

width_o	equ 05h
glyph_o		db 00000000
		db 00000000
		db 01110000
		db 10001000
		db 10001000
		db 10001000
		db 01110000
		db 00000000
		db 00000000

width_p	equ 05h
glyph_p		db 00000000
		db 00000000
		db 11110000
		db 10001000
		db 10001000
		db 10001000
		db 11110000
		db 10000000
		db 10000000

width_q	equ 05h
glyph_q		db 00000000
		db 00000000
		db 01111000
		db 10001000
		db 10001000
		db 10001000
		db 01111000
		db 00001000
		db 00001000

width_r	equ 04h
glyph_r		db 00000000
		db 00000000
		db 10110000
		db 11000000
		db 10000000
		db 10000000
		db 10000000
		db 00000000
		db 00000000

width_s	equ 05h
glyph_s		db 00000000
		db 00000000
		db 01110000
		db 10000000
		db 01110000
		db 00001000
		db 01110000
		db 00000000
		db 00000000

width_t	equ 04h
glyph_t		db 00000000
		db 01000000
		db 11100000
		db 01000000
		db 01000000
		db 01000000
		db 00110000
		db 00000000
		db 00000000

width_u	equ 05h
glyph_u		db 00000000
		db 00000000
		db 10001000
		db 10001000
		db 10001000
		db 10011000
		db 01101000
		db 00000000
		db 00000000

width_v	equ 05h
glyph_v		db 00000000
		db 00000000
		db 10001000
		db 10001000
		db 10001000
		db 01010000
		db 00100000
		db 00000000
		db 00000000

width_w	equ 07h
glyph_w		db 00000000
		db 00000000
		db 10010010
		db 10010010
		db 10010010
		db 10010010
		db 01101100
		db 00000000
		db 00000000

width_x	equ 05h
glyph_x		db 00000000
		db 00000000
		db 10001000
		db 01010000
		db 00100000
		db 01010000
		db 10001000
		db 00000000
		db 00000000

width_y	equ 05h
glyph_y		db 00000000
		db 00000000
		db 10001000
		db 10001000
		db 10001000
		db 10001000
		db 01111000
		db 00001000
		db 01110000

width_z	equ 05h
glyph_z		db 00000000
		db 00000000
		db 11111000
		db 00010000
		db 00100000
		db 01000000
		db 11111000
		db 00000000
		db 00000000

width_lbrace	equ 04h
glyph_lbrace	db 00110000
		db 01000000
		db 01000000
		db 10000000
		db 01000000
		db 01000000
		db 00110000
		db 00000000
		db 00000000

width_vbar	equ 01h
glyph_vbar	db 10000000
		db 10000000
		db 10000000
		db 00000000
		db 10000000
		db 10000000
		db 10000000
		db 00000000
		db 00000000

width_rbrace	equ 04h
glyph_rbrace	db 11000000
		db 00100000
		db 00100000
		db 00010000
		db 00100000
		db 00100000
		db 11000000
		db 00000000
		db 00000000

width_tilde	equ 07h
glyph_tilde	db 01110110
		db 11011100
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000
		db 00000000

width_bullet	equ 06h
glyph_bullet	db 00000000
		db 01111000
		db 11111100
		db 11111100
		db 11111100
		db 01111000
		db 00000000
		db 00000000
		db 00000000

width_cross	equ 07h
glyph_cross	db 00000000
		db 01000100
		db 00101000
		db 00010000
		db 00101000
		db 01000100
		db 00000000
		db 00000000
		db 00000000

width_tick	equ 07h
glyph_tick	db 00000000
		db 00000010
		db 00000100
		db 10001000
		db 01010000
		db 00100000
		db 00000000
		db 00000000
		db 00000000

width_uparrow	equ 07h
glyph_uparrow	db 00010000
		db 00111000
		db 01111100
		db 11111110
		db 00111000
		db 00111000
		db 00111000
		db 00000000
		db 00000000

width_downarrow	equ 07h
glyph_downarrow	db 00111000
		db 00111000
		db 00111000
		db 11111110
		db 01111100
		db 00111000
		db 00010000
		db 00000000
		db 00000000


width_rightarrow	equ 07h
glyph_rightarrow	db 00010000
		db 00011000
		db 11111100
		db 11111110
		db 11111100
		db 00011000
		db 00010000
		db 00000000
		db 00000000

width_leftarrow	equ 07h
glyph_leftarrow	db 00010000
		db 00110000
		db 01111110
		db 11111110
		db 01111110
		db 00110000
		db 00010000
		db 00000000
		db 00000000

width_solidbox	equ 07h
glyph_solidbox	db 11111110
		db 11111110
		db 11111110
		db 11111110
		db 11111110
		db 11111110
		db 11111110
		db 00000000
		db 00000000

width_cursor	equ 03h
glyph_cursor	db 10100000
		db 01000000
		db 01000000
		db 01000000
		db 01000000
		db 01000000
		db 01000000
		db 01000000
		db 10100000
	end
