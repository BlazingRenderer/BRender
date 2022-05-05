
option expr32
option casemap:none

; Begin of file host.h
; Begin of file D:\CrocPC\croc\source\brender\inc\brender.h
__BRENDER__		EQU		000010300h
; Begin of file D:\CrocPC\croc\source\brender\inc\compiler.h
br_int_32		TYPEDEF		SDWORD

br_uint_32		TYPEDEF		DWORD

br_int_16		TYPEDEF		SWORD

br_uint_16		TYPEDEF		WORD

br_int_8		TYPEDEF		SBYTE

br_uint_8		TYPEDEF		BYTE

br_size_t		TYPEDEF		DWORD

br_boolean		TYPEDEF		SDWORD

BR_TRUE		EQU		1t
BR_FALSE		EQU		0t
br_float		TYPEDEF		REAL4

BR_HAS_FAR		EQU		0t
BR_ENDIAN_BIG		EQU		0t
BR_ENDIAN_LITTLE		EQU		1t
DEBUG		EQU		0t
ADD_RCS_ID		EQU		0t
NULL		EQU		0t
; End of file D:\CrocPC\croc\source\brender\inc\compiler.h
; Begin of file D:\CrocPC\croc\source\brender\inc\brlimits.h
BR_MAX_NAME		EQU		256t
BR_MAX_LIGHTS		EQU		48t
BR_MAX_CLIP_PLANES		EQU		6t
BR_MAX_HORIZON_PLANES		EQU		6t
BR_MAX_RESOURCE_CLASSES		EQU		256t
MAX_CAMERA_DEPTH		EQU		16t
; End of file D:\CrocPC\croc\source\brender\inc\brlimits.h
; Begin of file D:\CrocPC\croc\source\brender\inc\brfile.h
@proto_0		TYPEDEF		PROTO STDCALL :PTR SBYTE, :PTR 
br_putline_cbfn		TYPEDEF		@proto_0

@proto_1		TYPEDEF		PROTO STDCALL :PTR br_uint_8, :br_size_t
br_mode_test_cbfn		TYPEDEF		@proto_1

@proto_2		TYPEDEF		PROTO STDCALL 
brfile_attributes_cbfn		TYPEDEF		@proto_2

@proto_3		TYPEDEF		PROTO STDCALL :PTR SBYTE, :br_size_t, :PTR br_mode_test_cbfn, :PTR SDWORD
brfile_open_read_cbfn		TYPEDEF		@proto_3

@proto_4		TYPEDEF		PROTO STDCALL :PTR SBYTE, :SDWORD
brfile_open_write_cbfn		TYPEDEF		@proto_4

@proto_5		TYPEDEF		PROTO STDCALL :PTR 
brfile_close_cbfn		TYPEDEF		@proto_5

@proto_6		TYPEDEF		PROTO STDCALL :PTR 
brfile_eof_cbfn		TYPEDEF		@proto_6

@proto_7		TYPEDEF		PROTO STDCALL :PTR 
brfile_getchr_cbfn		TYPEDEF		@proto_7

@proto_8		TYPEDEF		PROTO STDCALL :SDWORD, :PTR 
brfile_putchr_cbfn		TYPEDEF		@proto_8

@proto_9		TYPEDEF		PROTO STDCALL :PTR , :br_size_t, :DWORD, :PTR 
brfile_read_cbfn		TYPEDEF		@proto_9

@proto_10		TYPEDEF		PROTO STDCALL :PTR , :br_size_t, :DWORD, :PTR 
brfile_write_cbfn		TYPEDEF		@proto_10

@proto_11		TYPEDEF		PROTO STDCALL :PTR SBYTE, :br_size_t, :PTR 
brfile_getline_cbfn		TYPEDEF		@proto_11

@proto_12		TYPEDEF		PROTO STDCALL :PTR SBYTE, :PTR 
brfile_putline_cbfn		TYPEDEF		@proto_12

@proto_13		TYPEDEF		PROTO STDCALL :br_size_t, :PTR 
brfile_advance_cbfn		TYPEDEF		@proto_13

@t_0		TYPEDEF		PTR SBYTE
@t_1		TYPEDEF		PTR brfile_attributes_cbfn
@t_2		TYPEDEF		PTR brfile_open_read_cbfn
@t_3		TYPEDEF		PTR brfile_open_write_cbfn
@t_4		TYPEDEF		PTR brfile_close_cbfn
@t_5		TYPEDEF		PTR brfile_eof_cbfn
@t_6		TYPEDEF		PTR brfile_getchr_cbfn
@t_7		TYPEDEF		PTR brfile_putchr_cbfn
@t_8		TYPEDEF		PTR brfile_read_cbfn
@t_9		TYPEDEF		PTR brfile_write_cbfn
@t_10		TYPEDEF		PTR brfile_getline_cbfn
@t_11		TYPEDEF		PTR brfile_putline_cbfn
@t_12		TYPEDEF		PTR brfile_advance_cbfn
br_filesystem		STRUCT 4t
identifier		@t_0		?
attributes		@t_1		?
open_read		@t_2		?
open_write		@t_3		?
close		@t_4		?
eof		@t_5		?
getchr		@t_6		?
putchr		@t_7		?
read		@t_8		?
write		@t_9		?
getline		@t_10		?
putline		@t_11		?
advance		@t_12		?
br_filesystem		ENDS

BR_FS_ATTR_READABLE		EQU		1t
BR_FS_ATTR_WRITEABLE		EQU		2t
BR_FS_ATTR_HAS_TEXT		EQU		4t
BR_FS_ATTR_HAS_BINARY		EQU		8t
BR_FS_ATTR_HAS_ADVANCE		EQU		16t

BR_FS_MODE_BINARY		EQU		0t
BR_FS_MODE_TEXT		EQU		1t
BR_FS_MODE_UNKNOWN		EQU		2t

BR_MAX_FILE_MAGICS		EQU		16t
BR_EOF		EQU		0ffffffffh
; End of file D:\CrocPC\croc\source\brender\inc\brfile.h
; Begin of file D:\CrocPC\croc\source\brender\inc\brmem.h
@proto_14		TYPEDEF		PROTO STDCALL :br_size_t, :br_uint_8
brmem_allocate_cbfn		TYPEDEF		@proto_14

@proto_15		TYPEDEF		PROTO STDCALL :PTR 
brmem_free_cbfn		TYPEDEF		@proto_15

@proto_16		TYPEDEF		PROTO STDCALL :br_uint_8
brmem_inquire_cbfn		TYPEDEF		@proto_16

@proto_17		TYPEDEF		PROTO STDCALL :br_uint_8
brmem_align_cbfn		TYPEDEF		@proto_17

@t_13		TYPEDEF		PTR SBYTE
@t_14		TYPEDEF		PTR brmem_allocate_cbfn
@t_15		TYPEDEF		PTR brmem_free_cbfn
@t_16		TYPEDEF		PTR brmem_inquire_cbfn
@t_17		TYPEDEF		PTR brmem_align_cbfn
br_allocator		STRUCT 4t
identifier		@t_13		?
allocate		@t_14		?
free		@t_15		?
inquire		@t_16		?
_align		@t_17		?
br_allocator		ENDS

BR_MEMORY_SCRATCH		EQU		1t
BR_MEMORY_PIXELMAP		EQU		2t
BR_MEMORY_PIXELS		EQU		3t
BR_MEMORY_VERTICES		EQU		4t
BR_MEMORY_FACES		EQU		5t
BR_MEMORY_GROUPS		EQU		6t
BR_MEMORY_MODEL		EQU		7t
BR_MEMORY_MATERIAL		EQU		8t
BR_MEMORY_MATERIAL_INDEX		EQU		9t
BR_MEMORY_ACTOR		EQU		10t
BR_MEMORY_PREPARED_VERTICES		EQU		11t
BR_MEMORY_PREPARED_FACES		EQU		12t
BR_MEMORY_LIGHT		EQU		13t
BR_MEMORY_CAMERA		EQU		14t
BR_MEMORY_BOUNDS		EQU		15t
BR_MEMORY_CLIP_PLANE		EQU		16t
BR_MEMORY_STRING		EQU		17t
BR_MEMORY_REGISTRY		EQU		18t
BR_MEMORY_TRANSFORM		EQU		19t
BR_MEMORY_RESOURCE_CLASS		EQU		20t
BR_MEMORY_FILE		EQU		21t
BR_MEMORY_ANCHOR		EQU		22t
BR_MEMORY_POOL		EQU		23t
BR_MEMORY_RENDER_MATERIAL		EQU		24t
BR_MEMORY_DATAFILE		EQU		25t
BR_MEMORY_IMAGE		EQU		26t
BR_MEMORY_IMAGE_ARENA		EQU		27t
BR_MEMORY_IMAGE_SECTIONS		EQU		28t
BR_MEMORY_IMAGE_NAMES		EQU		29t
BR_MEMORY_EXCEPTION_HANDLER		EQU		30t
BR_MEMORY_RENDER_DATA		EQU		31t
BR_MEMORY_TOKEN		EQU		32t
BR_MEMORY_TOKEN_MAP		EQU		33t
BR_MEMORY_OBJECT		EQU		34t
BR_MEMORY_OBJECT_DATA		EQU		35t
BR_MEMORY_DRIVER		EQU		36t
BR_MEMORY_LEXER		EQU		37t
BR_MEMORY_OBJECT_LIST		EQU		38t
BR_MEMORY_OBJECT_LIST_ENTRY		EQU		39t
BR_MEMORY_ENABLED_ACTORS		EQU		40t
BR_MEMORY_FMT_RESULTS		EQU		41t
BR_MEMORY_PREPARED_MODEL		EQU		42t
BR_MEMORY_ORDER_TABLE		EQU		43t
BR_MEMORY_TOKEN_VALUE		EQU		44t
BR_MEMORY_TOKEN_TEMPLATE		EQU		45t
BR_MEMORY_FREE		EQU		127t
BR_MEMORY_APPLICATION		EQU		128t
BR_MEMORY_MAX		EQU		256t

@proto_18		TYPEDEF		PROTO STDCALL :PTR , :br_uint_8, :br_size_t
br_resourcefree_cbfn		TYPEDEF		@proto_18

@t_18		TYPEDEF		PTR SBYTE
@t_19		TYPEDEF		PTR br_resourcefree_cbfn
br_resource_class		STRUCT 4t
reserved		br_uint_32		?
identifier		@t_18		?
res_class		br_uint_8		?
free_cb		@t_19		?
alignment		br_uint_32		?
br_resource_class		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\brmem.h
; Begin of file D:\CrocPC\croc\source\brender\fw\brstdlib.h
; Begin of file d:\VC98\INCLUDE\stdarg.h
va_list		TYPEDEF		PTR SBYTE

; End of file d:\VC98\INCLUDE\stdarg.h
; End of file D:\CrocPC\croc\source\brender\fw\brstdlib.h
; Begin of file D:\CrocPC\croc\source\brender\inc\brdiag.h
@proto_19		TYPEDEF		PROTO STDCALL :PTR SBYTE
br_diag_warning_cbfn		TYPEDEF		@proto_19

@proto_20		TYPEDEF		PROTO STDCALL :PTR SBYTE
br_diag_failure_cbfn		TYPEDEF		@proto_20

@t_20		TYPEDEF		PTR SBYTE
@t_21		TYPEDEF		PTR br_diag_warning_cbfn
@t_22		TYPEDEF		PTR br_diag_failure_cbfn
br_diaghandler		STRUCT 4t
identifier		@t_20		?
warning		@t_21		?
failure		@t_22		?
br_diaghandler		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\brdiag.h
; Begin of file D:\CrocPC\croc\source\brender\inc\brerror.h
br_error		TYPEDEF		br_uint_32

BRE_OK		EQU		000000000h
BR_ECLASS_NONE		EQU		000000000h
BR_ECLASS_MISC		EQU		000001000h
BR_ECLASS_APPLICATION		EQU		000002000h
BR_ECLASS_DATABASE		EQU		000003000h
BR_ECLASS_RENDERER		EQU		000004000h
BR_ECLASS_FILE		EQU		000005000h
BR_ECLASS_TOKEN		EQU		000006000h
BR_ECLASS_MATH		EQU		000007000h
BR_ECLASS_DOSIO		EQU		000008000h
BR_ECLASS_HOST		EQU		000009000h
BR_ECLASS_DEVICE		EQU		00000a000h
BRE_UNKNOWN		EQU		000001001h
BRE_FAIL		EQU		000001002h
BRE_NO_MEMORY		EQU		000001003h
BRE_OVERFLOW		EQU		000001004h
BRE_UNDERFLOW		EQU		000001005h
BRE_NOT_ACTIVE		EQU		000001006h
BRE_ALLREADY_ACTIVE		EQU		000001007h
BRE_UNSUPPORTED		EQU		000001008h
BRE_INVALID		EQU		000001009h
BRE_DEV_FAIL		EQU		00000a001h
BRE_DEV_HARDWARE_NOT_PRESENT		EQU		00000a002h
BRE_DEV_NO_MEMORY		EQU		00000a003h
BRE_DEV_NO_VIDEO_MEMORY		EQU		00000a004h
BRE_DEV_UNSUPPORTED		EQU		00000a005h
BRE_DEV_HARDWARE_SET		EQU		00000a006h
BRE_DEV_HARDWARE_NOT_READY		EQU		00000a007h
BRE_DEV_NOT_FOUND		EQU		00000a008h
BRE_DEV_REFUSED		EQU		00000a009h
BRE_DEV_LOCK_FAIL		EQU		00000a00ah
BRE_DEV_OUTPUT_UNAVAILABLE		EQU		00000a00bh
BRE_DEV_ALREADY_INIT		EQU		00000a00ch
BRE_DEV_DIRECTDRAW_ERROR		EQU		00000a00dh
; End of file D:\CrocPC\croc\source\brender\inc\brerror.h
; Begin of file D:\CrocPC\croc\source\brender\inc\fixed.h
INLINE_FIXED		EQU		1t
br_fixed_ls		TYPEDEF		SDWORD

br_fixed_lsf		TYPEDEF		SWORD

br_fixed_lu		TYPEDEF		DWORD

br_fixed_luf		TYPEDEF		WORD

br_fixed_ss		TYPEDEF		SWORD

br_fixed_ssf		TYPEDEF		SBYTE

br_fixed_su		TYPEDEF		WORD

br_fixed_suf		TYPEDEF		BYTE

BR_ONE_LS		EQU		65536t
BR_ONE_LSF		EQU		32768t
BR_ONE_LU		EQU		65536t
BR_ONE_LUF		EQU		65536t
BR_ONE_SS		EQU		256t
BR_ONE_SSF		EQU		128t
BR_ONE_SU		EQU		256t
BR_ONE_SUF		EQU		256t
; End of file D:\CrocPC\croc\source\brender\inc\fixed.h
; Begin of file D:\CrocPC\croc\source\brender\inc\scalar.h
BASED_FIXED		EQU		1t
BASED_FLOAT		EQU		0t
br_scalar		TYPEDEF		br_fixed_ls

br_fraction		TYPEDEF		br_fixed_lsf

br_ufraction		TYPEDEF		br_fixed_luf

BR_SCALAR_EPSILON		EQU		000000001h
BR_SCALAR_MAX		EQU		07fffffffh
BR_SCALAR_MIN		EQU		080000000h
br_scalar_x		TYPEDEF		br_fixed_ls

br_fraction_x		TYPEDEF		br_fixed_lsf

br_ufraction_x		TYPEDEF		br_fixed_luf

br_scalar_f		TYPEDEF		REAL4

br_fraction_f		TYPEDEF		REAL4

br_ufraction_f		TYPEDEF		REAL4

; End of file D:\CrocPC\croc\source\brender\inc\scalar.h
; Begin of file D:\CrocPC\croc\source\brender\inc\colour.h
br_colour		TYPEDEF		DWORD

br_colour_range		STRUCT 4t
_low		br_colour		?
_high		br_colour		?
br_colour_range		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\colour.h
; Begin of file D:\CrocPC\croc\source\brender\inc\matrix.h
br_matrix23		STRUCT 4t
m		br_scalar		3t DUP (2t DUP (?))
br_matrix23		ENDS

br_matrix23_f		STRUCT 4t
m		br_float		3t DUP (2t DUP (?))
br_matrix23_f		ENDS

br_matrix23_x		STRUCT 4t
m		br_fixed_ls		3t DUP (2t DUP (?))
br_matrix23_x		ENDS

br_matrix34		STRUCT 4t
m		br_scalar		4t DUP (3t DUP (?))
br_matrix34		ENDS

br_matrix34_f		STRUCT 4t
m		br_float		4t DUP (3t DUP (?))
br_matrix34_f		ENDS

br_matrix34_x		STRUCT 4t
m		br_fixed_ls		4t DUP (3t DUP (?))
br_matrix34_x		ENDS

br_matrix4		STRUCT 4t
m		br_scalar		4t DUP (4t DUP (?))
br_matrix4		ENDS

br_matrix4_f		STRUCT 4t
m		br_float		4t DUP (4t DUP (?))
br_matrix4_f		ENDS

br_matrix4_x		STRUCT 4t
m		br_fixed_ls		4t DUP (4t DUP (?))
br_matrix4_x		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\matrix.h
; Begin of file D:\CrocPC\croc\source\brender\inc\vector.h
br_vector2		STRUCT 4t
v		br_scalar		2t DUP (?)
br_vector2		ENDS

br_vector3		STRUCT 4t
v		br_scalar		3t DUP (?)
br_vector3		ENDS

br_vector4		STRUCT 4t
v		br_scalar		4t DUP (?)
br_vector4		ENDS

br_vector2_x		STRUCT 4t
v		br_fixed_ls		2t DUP (?)
br_vector2_x		ENDS

br_vector3_x		STRUCT 4t
v		br_fixed_ls		3t DUP (?)
br_vector3_x		ENDS

br_vector4_x		STRUCT 4t
v		br_fixed_ls		4t DUP (?)
br_vector4_x		ENDS

br_vector2_f		STRUCT 4t
v		br_float		2t DUP (?)
br_vector2_f		ENDS

br_vector3_f		STRUCT 4t
v		br_float		3t DUP (?)
br_vector3_f		ENDS

br_vector4_f		STRUCT 4t
v		br_float		4t DUP (?)
br_vector4_f		ENDS

br_vector2_i		STRUCT 4t
v		br_int_32		2t DUP (?)
br_vector2_i		ENDS

br_vector3_i		STRUCT 4t
v		br_int_32		3t DUP (?)
br_vector3_i		ENDS

br_vector4_i		STRUCT 4t
v		br_int_32		4t DUP (?)
br_vector4_i		ENDS

br_fvector2		STRUCT ; 4t H2INCFIX
v		br_fraction		2t DUP (?)
br_fvector2		ENDS

br_fvector3		STRUCT ; 4t H2INCFIX
v		br_fraction		3t DUP (?)
br_fvector3		ENDS

br_fvector4		STRUCT ; 4t H2INCFIX
v		br_fraction		4t DUP (?)
br_fvector4		ENDS

br_fvector2_f		STRUCT ; 4t H2INCFIX
v		br_fraction_f		2t DUP (?)
br_fvector2_f		ENDS

br_fvector3_f		STRUCT ; 4t H2INCFIX
v		br_fraction_f		3t DUP (?)
br_fvector3_f		ENDS

br_fvector4_f		STRUCT ; 4t H2INCFIX
v		br_fraction		4t DUP (?)
br_fvector4_f		ENDS

br_fvector2_x		STRUCT ; 4t H2INCFIX
v		br_fraction_x		2t DUP (?)
br_fvector2_x		ENDS

br_fvector3_x		STRUCT ; 4t H2INCFIX
v		br_fraction_x		3t DUP (?)
br_fvector3_x		ENDS

br_fvector4_x		STRUCT ; 4t H2INCFIX
v		br_fraction_x		4t DUP (?)
br_fvector4_x		ENDS

br_bounds2		STRUCT 4t
min		br_vector2		<>
max		br_vector2		<>
br_bounds2		ENDS

br_bounds3		STRUCT 4t
min		br_vector3		<>
max		br_vector3		<>
br_bounds3		ENDS

br_bounds4		STRUCT 4t
min		br_vector4		<>
max		br_vector4		<>
br_bounds4		ENDS

br_bounds2_f		STRUCT 4t
min		br_vector2_f		<>
max		br_vector2_f		<>
br_bounds2_f		ENDS

br_bounds3_f		STRUCT 4t
min		br_vector3_f		<>
max		br_vector3_f		<>
br_bounds3_f		ENDS

br_bounds4_f		STRUCT 4t
min		br_vector4_f		<>
max		br_vector4_f		<>
br_bounds4_f		ENDS

br_bounds2_x		STRUCT 4t
min		br_vector2_x		<>
max		br_vector2_x		<>
br_bounds2_x		ENDS

br_bounds3_x		STRUCT 4t
min		br_vector3_x		<>
max		br_vector3_x		<>
br_bounds3_x		ENDS

br_bounds4_x		STRUCT 4t
min		br_vector4_x		<>
max		br_vector4_x		<>
br_bounds4_x		ENDS

br_bounds2_i		STRUCT 4t
min		br_vector2_i		<>
max		br_vector2_i		<>
br_bounds2_i		ENDS

br_bounds3_i		STRUCT 4t
min		br_vector3_i		<>
max		br_vector3_i		<>
br_bounds3_i		ENDS

br_bounds4_i		STRUCT 4t
min		br_vector4_i		<>
max		br_vector4_i		<>
br_bounds4_i		ENDS

br_bounds		TYPEDEF		br_bounds3

br_bounds2i		TYPEDEF		br_bounds2_i

br_bounds3i		TYPEDEF		br_bounds3_i

br_bounds4i		TYPEDEF		br_bounds4_i

; End of file D:\CrocPC\croc\source\brender\inc\vector.h
; Begin of file D:\CrocPC\croc\source\brender\inc\angles.h
br_angle		TYPEDEF		br_fixed_luf

BR_EULER_FIRST		EQU		3t
BR_EULER_FIRST_X		EQU		0t
BR_EULER_FIRST_Y		EQU		1t
BR_EULER_FIRST_Z		EQU		2t
BR_EULER_PARITY		EQU		4t
BR_EULER_PARITY_EVEN		EQU		0t
BR_EULER_PARITY_ODD		EQU		4t
BR_EULER_REPEAT		EQU		8t
BR_EULER_REPEAT_NO		EQU		0t
BR_EULER_REPEAT_YES		EQU		8t
BR_EULER_FRAME		EQU		16t
BR_EULER_FRAME_STATIC		EQU		0t
BR_EULER_FRAME_ROTATING		EQU		16t

BR_EULER_XYZ_S		EQU		0t
BR_EULER_XYX_S		EQU		8t
BR_EULER_XZY_S		EQU		4t
BR_EULER_XZX_S		EQU		12t
BR_EULER_YZX_S		EQU		1t
BR_EULER_YZY_S		EQU		9t
BR_EULER_YXZ_S		EQU		5t
BR_EULER_YXY_S		EQU		13t
BR_EULER_ZXY_S		EQU		2t
BR_EULER_ZXZ_S		EQU		10t
BR_EULER_ZYX_S		EQU		6t
BR_EULER_ZYZ_S		EQU		14t
BR_EULER_ZYX_R		EQU		16t
BR_EULER_XYX_R		EQU		24t
BR_EULER_YZX_R		EQU		20t
BR_EULER_XZX_R		EQU		28t
BR_EULER_XZY_R		EQU		17t
BR_EULER_YZY_R		EQU		25t
BR_EULER_ZXY_R		EQU		21t
BR_EULER_YXY_R		EQU		29t
BR_EULER_YXZ_R		EQU		18t
BR_EULER_ZXZ_R		EQU		26t
BR_EULER_XYZ_R		EQU		22t
BR_EULER_ZYZ_R		EQU		30t

br_euler		STRUCT 4t
_a		br_angle		?
_b		br_angle		?
_c		br_angle		?
order		br_uint_8		?
br_euler		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\angles.h
; Begin of file D:\CrocPC\croc\source\brender\inc\quat.h
br_quat		STRUCT 4t
x		br_scalar		?
y		br_scalar		?
z		br_scalar		?
w		br_scalar		?
br_quat		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\quat.h
; Begin of file D:\CrocPC\croc\source\brender\inc\transfrm.h
BR_TRANSFORM_MATRIX34		EQU		0t
BR_TRANSFORM_MATRIX34_LP		EQU		1t
BR_TRANSFORM_QUAT		EQU		2t
BR_TRANSFORM_EULER		EQU		3t
BR_TRANSFORM_LOOK_UP		EQU		4t
BR_TRANSFORM_TRANSLATION		EQU		5t
BR_TRANSFORM_IDENTITY		EQU		6t
BR_TRANSFORM_MAX		EQU		7t

@tag_4		STRUCT 4t
e		br_euler		<>
_pad		br_scalar		7t DUP (?)
t		br_vector3		<>
@tag_4		ENDS

@tag_5		STRUCT 4t
q		br_quat		<>
_pad		br_scalar		5t DUP (?)
t		br_vector3		<>
@tag_5		ENDS

@tag_6		STRUCT 4t
look		br_vector3		<>
up		br_vector3		<>
_pad		br_scalar		3t DUP (?)
t		br_vector3		<>
@tag_6		ENDS

@tag_7		STRUCT 4t
_pad		br_scalar		9t DUP (?)
t		br_vector3		<>
@tag_7		ENDS

@tag_3		UNION
mat		br_matrix34		<>
euler		@tag_4		<>
quat		@tag_5		<>
look_up		@tag_6		<>
translate		@tag_7		<>
@tag_3		ENDS

br_transform		STRUCT 4t
_type		br_uint_16		?
t		@tag_3		<>
br_transform		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\transfrm.h
; Begin of file D:\CrocPC\croc\source\brender\inc\pixelmap.h
BR_PMT_INDEX_1		EQU		0t
BR_PMT_INDEX_2		EQU		1t
BR_PMT_INDEX_4		EQU		2t
BR_PMT_INDEX_8		EQU		3t
BR_PMT_RGB_555		EQU		4t
BR_PMT_RGB_565		EQU		5t
BR_PMT_RGB_888		EQU		6t
BR_PMT_RGBX_888		EQU		7t
BR_PMT_RGBA_8888		EQU		8t
BR_PMT_YUYV_8888		EQU		9t
BR_PMT_YUV_888		EQU		10t
BR_PMT_DEPTH_16		EQU		11t
BR_PMT_DEPTH_32		EQU		12t
BR_PMT_ALPHA_8		EQU		13t
BR_PMT_INDEXA_88		EQU		14t
BR_PMT_NORMAL_INDEX_8		EQU		15t
BR_PMT_NORMAL_XYZ		EQU		16t
BR_PMT_BGR_555		EQU		17t
BR_PMT_RGBA_4444		EQU		18t
BR_PMT_RBG_bab		EQU		19t
BR_PMT_RBG_1aba		EQU		20t
BR_PMT_RGB_332		EQU		21t
BR_PMT_DEPTH_8		EQU		22t
BR_PMT_ARGB_8888		EQU		23t
BR_PMT_ALPHA_4		EQU		24t
BR_PMT_AINDEX_44		EQU		25t
BR_PMT_AINDEX_88		EQU		26t
BR_PMT_DEPTH_15		EQU		27t
BR_PMT_DEPTH_31		EQU		28t
BR_PMT_DEPTH_FP16		EQU		29t
BR_PMT_DEPTH_FP15		EQU		30t
BR_PMT_MAX		EQU		31t

BR_PMF_NO_ACCESS		EQU		1t
BR_PMF_LINEAR		EQU		2t
BR_PMF_ROW_WHOLEPIXELS		EQU		4t
BR_PMF_PIXELS_NEAR		EQU		8t
BR_PMF_PIXELS_NEAR_ALIAS		EQU		16t
BR_PMF_KEYED_TRANSPARENCY		EQU		32t
BR_PMF_KEEP_ORIGINAL		EQU		64t
BR_PMF_TILED		EQU		128t

BR_PM_TILE_SIZE		EQU		2t
@t_23		TYPEDEF		PTR SBYTE
@t_24		TYPEDEF		PTR 
@t_25		TYPEDEF		PTR br_pixelmap
@t_26		TYPEDEF		PTR 
@t_27		TYPEDEF		PTR 
br_pixelmap		STRUCT 4t
_reserved		br_uint_32		?
identifier		@t_23		?
pixels		@t_24		?
pixels_qualifier		br_uint_32		?
map		@t_25		?
src_key		br_colour_range		<>
dst_key		br_colour_range		<>
key		br_uint_32		?
row_bytes		br_int_16		?
mip_offset		br_int_16		?
_type		br_uint_8		?
flags		br_uint_8		?
copy_function		br_uint_16		?
base_x		br_uint_16		?
base_y		br_uint_16		?
_width		br_uint_16		?
height		br_uint_16		?
origin_x		br_int_16		?
origin_y		br_int_16		?
user		@t_26		?
stored		@t_27		?
br_pixelmap		ENDS

BR_PMAF_NORMAL		EQU		0t
BR_PMAF_INVERTED		EQU		1t
BR_PMAF_NO_PIXELS		EQU		2t

BR_PMCHAN_INDEX		EQU		1t
BR_PMCHAN_RGB		EQU		2t
BR_PMCHAN_DEPTH		EQU		4t
BR_PMCHAN_ALPHA		EQU		8t
BR_PMCHAN_YUV		EQU		16t
BR_PMCHAN_VECTOR		EQU		32t

BR_PMMATCH_OFFSCREEN		EQU		0t
BR_PMMATCH_DEPTH_16		EQU		1t
BR_PMMATCH_DEPTH		EQU		2t
BR_PMMATCH_HIDDEN		EQU		3t
BR_PMMATCH_HIDDEN_BUFFER		EQU		4t
BR_PMMATCH_NO_RENDER		EQU		5t
BR_PMMATCH_DEPTH_8		EQU		6t
BR_PMMATCH_DEPTH_32		EQU		7t
BR_PMMATCH_DEPTH_15		EQU		8t
BR_PMMATCH_DEPTH_31		EQU		9t
BR_PMMATCH_DEPTH_FP15		EQU		10t
BR_PMMATCH_DEPTH_FP16		EQU		11t
BR_PMMATCH_MAX		EQU		12t

BR_PMCOPY_NORMAL		EQU		0t
BR_PMCOPY_SRC_KEYED		EQU		1t
BR_PMCOPY_DST_KEYED		EQU		2t

br_point		STRUCT 4t
x		br_int_32		?
y		br_int_32		?
br_point		ENDS

br_rectangle		STRUCT 4t
x		br_int_32		?
y		br_int_32		?
w		br_int_32		?
h		br_int_32		?
br_rectangle		ENDS

BR_CLIP_REJECT		EQU		0t
BR_CLIP_PARTIAL		EQU		1t
BR_CLIP_ACCEPT		EQU		2t

br_clip_result		TYPEDEF		SDWORD

BR_QUANTIZE_RGB		EQU		0t
BR_QUANTIZE_YIQ		EQU		1t
BR_QUANTIZE_MAX		EQU		2t

; End of file D:\CrocPC\croc\source\brender\inc\pixelmap.h
; Begin of file D:\CrocPC\croc\source\brender\inc\brfont.h
@t_28		TYPEDEF		PTR br_int_8
@t_29		TYPEDEF		PTR br_uint_16
@t_30		TYPEDEF		PTR br_uint_8
br_font		STRUCT 4t
flags		br_uint_32		?
glyph_x		br_uint_16		?
glyph_y		br_uint_16		?
spacing_x		br_int_16		?
spacing_y		br_int_16		?
_width		@t_28		?
encoding		@t_29		?
glyphs		@t_30		?
br_font		ENDS

BR_FONTF_PROPORTIONAL		EQU		1t
EXTERNDEF		C BrFontFixed3x5:PTR br_font

EXTERNDEF		C BrFontProp4x6:PTR br_font

EXTERNDEF		C BrFontProp7x9:PTR br_font

; End of file D:\CrocPC\croc\source\brender\inc\brfont.h
; Begin of file D:\CrocPC\croc\source\brender\inc\token.h
BR_TOKEN_ENUM		EQU		1t
BR_NULL_TOKEN		EQU		0t
BR_MAX_IDENTIFIER		EQU		64t
; Begin of file D:\CrocPC\croc\source\brender\fw\pretok.h
BRT_ABGR_1_5_5_5		EQU		319t
BRT_ABGR_4_4_4_4		EQU		335t
BRT_ABGR_8_8_8_8		EQU		331t
BRT_ACCEPT		EQU		276t
BRT_AINDEX_4_4		EQU		345t
BRT_AINDEX_8_8		EQU		344t
BRT_ALPHA		EQU		85t
BRT_ALTERNATIVE_TO_DEFAULT_B		EQU		434t
BRT_ALWAYS		EQU		163t
BRT_AMBIENT_B_F		EQU		501t
BRT_AMBIENT_B_X		EQU		500t
BRT_AMBIENT_F		EQU		193t
BRT_AMBIENT_G_F		EQU		499t
BRT_AMBIENT_G_X		EQU		498t
BRT_AMBIENT_R_F		EQU		497t
BRT_AMBIENT_R_X		EQU		496t
BRT_AMBIENT_X		EQU		192t
BRT_ANGLE		EQU		13t
BRT_ANTIALIASING_T		EQU		298t
BRT_ANTIALIASING_TL		EQU		299t
BRT_ARGB_1_5_5_5		EQU		318t
BRT_ARGB_4_4_4_4		EQU		334t
BRT_ARGB_8_8_8_8		EQU		330t
BRT_ATTENUATION_C_F		EQU		222t
BRT_ATTENUATION_C_X		EQU		221t
BRT_ATTENUATION_L_F		EQU		220t
BRT_ATTENUATION_L_X		EQU		219t
BRT_ATTENUATION_Q_F		EQU		224t
BRT_ATTENUATION_Q_X		EQU		223t
BRT_A_4		EQU		462t
BRT_A_8		EQU		341t
BRT_BEST_QUALITY		EQU		380t
BRT_BEST_SPEED		EQU		291t
BRT_BGRA_4_4_4_4		EQU		336t
BRT_BGRA_8_8_8_8		EQU		332t
BRT_BGRX_8_8_8_8		EQU		328t
BRT_BGR_2_3_2		EQU		323t
BRT_BGR_5_6_5		EQU		321t
BRT_BGR_8_8_8		EQU		325t
BRT_BLENDER_PROGRAM_P		EQU		470t
BRT_BLEND_B		EQU		133t
BRT_BLEND_CONSTANT		EQU		457t
BRT_BLEND_DIMMED		EQU		456t
BRT_BLEND_MODE_T		EQU		452t
BRT_BLEND_MODE_TL		EQU		453t
BRT_BLEND_STANDARD		EQU		454t
BRT_BLEND_SUMMED		EQU		455t
BRT_BLUE		EQU		83t
BRT_BOOLEAN		EQU		2t
BRT_BOTTOM_TOP		EQU		67t
BRT_BOUNDS		EQU		122t
BRT_BOUNDS_B		EQU		255t
BRT_BRENDER_DEVELOPER_B		EQU		442t
BRT_BRENDER_DEVICES_STR		EQU		422t
BRT_BRENDER_DRIVERS_STR		EQU		420t
BRT_BRENDER_PATH_STR		EQU		419t
BRT_BRENDER_USE_CMOV_B		EQU		444t
BRT_BRENDER_USE_MMX_B		EQU		443t
BRT_BRENDER_VERSION_U32		EQU		356t
BRT_BUCKET_SORT		EQU		257t
BRT_BUCKET_SORT_DEFERRED		EQU		508t
BRT_BUFFER		EQU		433t
BRT_BUFFER_STORED		EQU		45t
BRT_BUMP_O		EQU		170t
BRT_BUMP_TVL		EQU		401t
BRT_CACHED_B		EQU		286t
BRT_CACHED_SIZE_U32		EQU		287t
BRT_CACHED_TOTAL_U32		EQU		288t
BRT_CAN_SHARE_B		EQU		110t
BRT_CLAMP		EQU		308t
BRT_CLIP		EQU		121t
BRT_CLIPPING_B		EQU		254t
BRT_CLIP_MAX_I32		EQU		105t
BRT_CLONE		EQU		102t
BRT_CLUT_MAX_I32		EQU		90t
BRT_CLUT_O		EQU		68t
BRT_COLOUR_B		EQU		135t
BRT_COLOUR_BUFFER_O		EQU		249t
BRT_COLOUR_B_F		EQU		507t
BRT_COLOUR_B_X		EQU		506t
BRT_COLOUR_G_F		EQU		505t
BRT_COLOUR_G_X		EQU		504t
BRT_COLOUR_I32		EQU		189t
BRT_COLOUR_MAP_1_O		EQU		478t
BRT_COLOUR_MAP_O		EQU		165t
BRT_COLOUR_MAP_TVL		EQU		396t
BRT_COLOUR_RGB		EQU		14t
BRT_COLOUR_R_F		EQU		503t
BRT_COLOUR_R_X		EQU		502t
BRT_COLOUR_SOURCE_T		EQU		180t
BRT_COLOUR_T		EQU		136t
BRT_COLOUR_TL		EQU		297t
BRT_COLOUR_WRITE_B		EQU		132t
BRT_COMPOSITING		EQU		492t
BRT_COMPOSITING_WITH_INCREASE		EQU		493t
BRT_COMPOSITING_WITH_REDUCTION		EQU		490t
BRT_CONSTANT_STRING		EQU		32t
BRT_COPY_BITS_F		EQU		371t
BRT_COPY_BITS_X		EQU		370t
BRT_COPY_DESTINATION_TVL		EQU		361t
BRT_COPY_F		EQU		363t
BRT_COPY_SOURCE_TVL		EQU		360t
BRT_COPY_X		EQU		362t
BRT_CREATOR_CSTR		EQU		56t
BRT_CULL		EQU		116t
BRT_CULL_B		EQU		251t
BRT_DDI_VERSION_U32		EQU		357t
BRT_DECAL_B		EQU		128t
BRT_DEFAULT		EQU		138t
BRT_DEFAULT_BASE_I32		EQU		282t
BRT_DEFAULT_DEVICE_STR		EQU		421t
BRT_DEFAULT_RANGE_I32		EQU		283t
BRT_DEPTH		EQU		84t
BRT_DEPTH_15		EQU		471t
BRT_DEPTH_31		EQU		472t
BRT_DEPTH_BUFFER_O		EQU		250t
BRT_DEPTH_FP15		EQU		473t
BRT_DEPTH_FP16		EQU		474t
BRT_DEPTH_TEST_T		EQU		155t
BRT_DEPTH_TEST_TL		EQU		304t
BRT_DEPTH_WRITE_B		EQU		131t
BRT_DESTINATION_O		EQU		114t
BRT_DEVICE		EQU		37t
BRT_DEVICE_CLUT		EQU		41t
BRT_DEVICE_PIXELMAP		EQU		40t
BRT_DIBSECTION		EQU		410t
BRT_DIFFUSE_F		EQU		195t
BRT_DIFFUSE_X		EQU		194t
BRT_DIRECT		EQU		208t
BRT_DIRECT3D_HAL		EQU		416t
BRT_DIRECT3D_RAMP		EQU		414t
BRT_DIRECT3D_RENDER_METHOD_T		EQU		413t
BRT_DIRECT3D_RGB		EQU		415t
BRT_DIRECTDRAW		EQU		411t
BRT_DIRECTDRAW_DEVICENAME_CSTR		EQU		412t
BRT_DIRECTION_V3_F		EQU		213t
BRT_DIRECTION_V3_X		EQU		214t
BRT_DIRECT_F		EQU		378t
BRT_DIRECT_X		EQU		377t
BRT_DITHER		EQU		436t
BRT_DITHER_B		EQU		469t
BRT_DITHER_COLOUR_B		EQU		129t
BRT_DITHER_COLOUR_T		EQU		293t
BRT_DITHER_COLOUR_TL		EQU		294t
BRT_DITHER_MAP_B		EQU		130t
BRT_DITHER_MAP_T		EQU		295t
BRT_DITHER_MAP_TL		EQU		296t
BRT_DONT_CARE		EQU		246t
BRT_DOUBLE_TEXTURE_B		EQU		480t
BRT_D_15		EQU		458t
BRT_D_16		EQU		338t
BRT_D_24		EQU		339t
BRT_D_31		EQU		459t
BRT_D_32		EQU		340t
BRT_D_FP15		EQU		460t
BRT_D_FP16		EQU		461t
BRT_ENABLE		EQU		119t
BRT_ENVIRONMENT_INFINITE		EQU		184t
BRT_ENVIRONMENT_LOCAL		EQU		183t
BRT_EQUAL		EQU		158t
BRT_EVEN_ODD		EQU		63t
BRT_FACILITY		EQU		38t
BRT_FACILITY_O		EQU		281t
BRT_FIXED		EQU		11t
BRT_FLOAT		EQU		12t
BRT_FOG_B		EQU		494t
BRT_FOG_MAX_F		EQU		153t
BRT_FOG_MAX_X		EQU		154t
BRT_FOG_MIN_F		EQU		151t
BRT_FOG_MIN_X		EQU		152t
BRT_FOG_RGB		EQU		150t
BRT_FOG_SCALE_U8		EQU		481t
BRT_FOG_T		EQU		149t
BRT_FOG_TL		EQU		303t
BRT_FONT_NORMAL_O		EQU		98t
BRT_FONT_NORMAL_P		EQU		97t
BRT_FORCE_BACK_B		EQU		495t
BRT_FORCE_FRONT_B		EQU		179t
BRT_FORMAT_T		EQU		113t
BRT_GEOMETRY		EQU		46t
BRT_GEOMETRY_LIGHTING		EQU		51t
BRT_GEOMETRY_MAP		EQU		185t
BRT_GEOMETRY_PRIMITIVES		EQU		48t
BRT_GEOMETRY_STORED		EQU		47t
BRT_GEOMETRY_V1_BUCKETS		EQU		50t
BRT_GEOMETRY_V1_MODEL		EQU		49t
BRT_GEOMETRY_X		EQU		186t
BRT_GEOMETRY_Y		EQU		187t
BRT_GEOMETRY_Z		EQU		188t
BRT_GREATER		EQU		156t
BRT_GREATER_OR_EQUAL		EQU		157t
BRT_GREEN		EQU		82t
BRT_HANDLE		EQU		354t
BRT_HEIGHT_I32		EQU		72t
BRT_HEIGHT_MAX_I32		EQU		74t
BRT_HEIGHT_MIN_I32		EQU		73t
BRT_HEIGHT_OPTIMUM_I32		EQU		392t
BRT_HEIGHT_T		EQU		389t
BRT_HEIGHT_TL		EQU		390t
BRT_HIDDEN		EQU		100t
BRT_HIDDEN_BUFFER		EQU		278t
BRT_HIDDEN_SURFACE		EQU		123t
BRT_HITHER_Z_F		EQU		437t
BRT_HITHER_Z_X		EQU		438t
BRT_HOST_MEMORY_U32		EQU		93t
BRT_IDENTIFIER_CSTR		EQU		54t
BRT_INDEX		EQU		80t
BRT_INDEXA_4_4		EQU		343t
BRT_INDEXA_8_8		EQU		342t
BRT_INDEXED_B		EQU		86t
BRT_INDEX_1		EQU		312t
BRT_INDEX_2		EQU		313t
BRT_INDEX_4		EQU		314t
BRT_INDEX_8		EQU		315t
BRT_INDEX_BASE_I32		EQU		139t
BRT_INDEX_BLEND_O		EQU		167t
BRT_INDEX_BLEND_TVL		EQU		398t
BRT_INDEX_FOG_O		EQU		394t
BRT_INDEX_FOG_TVL		EQU		402t
BRT_INDEX_LIGHT_O		EQU		169t
BRT_INDEX_LIGHT_TVL		EQU		400t
BRT_INDEX_RANGE_I32		EQU		140t
BRT_INDEX_SHADE_O		EQU		166t
BRT_INDEX_SHADE_TVL		EQU		397t
BRT_INI_STR		EQU		418t
BRT_INTEL		EQU		423t
BRT_INTEL_386		EQU		427t
BRT_INTEL_486		EQU		428t
BRT_INTEL_PENTIUM		EQU		429t
BRT_INTEL_PENTIUM_PRO		EQU		430t
BRT_INTENSITY_REDUCTION		EQU		489t
BRT_INTERLACE_B		EQU		468t
BRT_INT_16		EQU		7t
BRT_INT_32		EQU		9t
BRT_INT_8		EQU		5t
BRT_KEY_IS_TRANSPARENT		EQU		353t
BRT_LEFT_RIGHT		EQU		64t
BRT_LENGTH_PRESERVING		EQU		237t
BRT_LESS		EQU		161t
BRT_LESS_OR_EQUAL		EQU		160t
BRT_LIGHT		EQU		120t
BRT_LIGHTING_B		EQU		178t
BRT_LIGHT_MAX_I32		EQU		104t
BRT_LIGHT_VERTICES		EQU		273t
BRT_LINE		EQU		266t
BRT_LINEAR		EQU		147t
BRT_LPDIRECTDRAWSURFACE_P		EQU		441t
BRT_MAPPING_SOURCE_T		EQU		182t
BRT_MAP_ANTIALIASING_T		EQU		141t
BRT_MAP_ANTIALIASING_TL		EQU		300t
BRT_MAP_HEIGHT_LIMIT_1_T		EQU		476t
BRT_MAP_HEIGHT_LIMIT_T		EQU		310t
BRT_MAP_HEIGHT_LIMIT_TL		EQU		311t
BRT_MAP_INTERPOLATION_MAG_T		EQU		383t
BRT_MAP_INTERPOLATION_MAG_TL		EQU		384t
BRT_MAP_INTERPOLATION_MIN_T		EQU		385t
BRT_MAP_INTERPOLATION_MIN_TL		EQU		386t
BRT_MAP_INTERPOLATION_T		EQU		145t
BRT_MAP_INTERPOLATION_TL		EQU		301t
BRT_MAP_MATRIX_M23_F		EQU		200t
BRT_MAP_MATRIX_M23_X		EQU		201t
BRT_MAP_WIDTH_LIMIT_1_T		EQU		477t
BRT_MAP_WIDTH_LIMIT_T		EQU		305t
BRT_MAP_WIDTH_LIMIT_TL		EQU		306t
BRT_MATCH_TVL		EQU		359t
BRT_MATRIX		EQU		118t
BRT_MATRIX23_FIXED		EQU		25t
BRT_MATRIX23_FLOAT		EQU		28t
BRT_MATRIX34_FIXED		EQU		26t
BRT_MATRIX34_FLOAT		EQU		29t
BRT_MATRIX4_FIXED		EQU		27t
BRT_MATRIX4_FLOAT		EQU		30t
BRT_MAX_V2_F		EQU		233t
BRT_MAX_V2_I		EQU		231t
BRT_MAX_V2_X		EQU		232t
BRT_MEMORY_MAPPED_B		EQU		87t
BRT_MIN_V2_F		EQU		230t
BRT_MIN_V2_I		EQU		228t
BRT_MIN_V2_X		EQU		229t
BRT_MIP		EQU		142t
BRT_MIPS		EQU		425t
BRT_MIP_INTERPOLATION_T		EQU		148t
BRT_MIP_INTERPOLATION_TL		EQU		302t
BRT_MIP_MAX_U8		EQU		487t
BRT_MIP_MIN_U8		EQU		486t
BRT_MIP_MODIFIER_I8		EQU		485t
BRT_MIP_MOD_ABSOLUTE_B		EQU		488t
BRT_MIP_PER_PIXEL		EQU		143t
BRT_MIP_PER_PRIMITIVE		EQU		144t
BRT_MIRROR		EQU		309t
BRT_MODEL		EQU		177t
BRT_MODEL_TO_SCREEN_M4_F		EQU		247t
BRT_MODEL_TO_SCREEN_M4_X		EQU		248t
BRT_MODEL_TO_VIEW_HINT_T		EQU		236t
BRT_MODEL_TO_VIEW_M34_F		EQU		234t
BRT_MODEL_TO_VIEW_M34_X		EQU		235t
BRT_MODE_U32		EQU		94t
BRT_MODULATE_B		EQU		134t
BRT_MOTOROLA		EQU		426t
BRT_NEAREST		EQU		146t
BRT_NEVER		EQU		162t
BRT_NONE		EQU		1t
BRT_NOT_EQUAL		EQU		159t
BRT_NO_RENDER		EQU		379t
BRT_OBJECT		EQU		15t
BRT_OBJECT_CONTAINER		EQU		36t
BRT_OBJECT_LIST		EQU		33t
BRT_ODD_EVEN		EQU		62t
BRT_OFFSCREEN		EQU		101t
BRT_ONE_SIDED		EQU		173t
BRT_OPACITY_F		EQU		191t
BRT_OPACITY_SOURCE_T		EQU		181t
BRT_OPACITY_X		EQU		190t
BRT_ORIGIN_V2_I		EQU		96t
BRT_OUTPUT		EQU		125t
BRT_OUTPUT_FACILITY		EQU		39t
BRT_OUTPUT_FACILITY_O		EQU		95t
BRT_PARALLEL		EQU		242t
BRT_PARTIAL		EQU		275t
BRT_PARTS_TL		EQU		107t
BRT_PARTS_U32		EQU		112t
BRT_PERSPECTIVE		EQU		241t
BRT_PERSPECTIVE_B		EQU		127t
BRT_PERSPECTIVE_T		EQU		289t
BRT_PERSPECTIVE_TL		EQU		290t
BRT_PIXELMAP_MAX_I32		EQU		89t
BRT_PIXEL_BITS_I32		EQU		76t
BRT_PIXEL_CHANNELS_I32		EQU		78t
BRT_PIXEL_CHANNELS_TL		EQU		79t
BRT_PIXEL_GET_F		EQU		374t
BRT_PIXEL_GET_X		EQU		373t
BRT_PIXEL_SET_F		EQU		376t
BRT_PIXEL_SET_X		EQU		375t
BRT_PIXEL_TYPE_U8		EQU		75t
BRT_PLANE		EQU		225t
BRT_PLANE_V4_F		EQU		226t
BRT_PLANE_V4_X		EQU		227t
BRT_POINT		EQU		209t
BRT_POINTER		EQU		3t
BRT_POINTER_LIST		EQU		35t
BRT_POSITION_V3_F		EQU		211t
BRT_POSITION_V3_X		EQU		212t
BRT_POWERPC		EQU		424t
BRT_POWER_OF_2		EQU		351t
BRT_PREFER_SHARE_B		EQU		111t
BRT_PRIMITIVE		EQU		124t
BRT_PRIMITIVES		EQU		270t
BRT_PRIMITIVE_BLOCK_P		EQU		393t
BRT_PRIMITIVE_CUSTOM_P		EQU		206t
BRT_PRIMITIVE_FILTER_P		EQU		207t
BRT_PRIMITIVE_LIBRARY		EQU		52t
BRT_PRIMITIVE_LIBRARY_O		EQU		88t
BRT_PRIMITIVE_LIBRARY_OL		EQU		431t
BRT_PRIMITIVE_LIBRARY_P		EQU		115t
BRT_PRIMITIVE_STATE		EQU		53t
BRT_PRIVATE		EQU		349t
BRT_PRODUCT_CSTR		EQU		57t
BRT_PRODUCT_VERSION_CSTR		EQU		58t
BRT_QUAD		EQU		268t
BRT_QUAD_STRIP		EQU		467t
BRT_RED		EQU		81t
BRT_REGISTRY_STR		EQU		417t
BRT_REJECT		EQU		277t
BRT_RENDERER		EQU		43t
BRT_RENDERER_FACILITY		EQU		42t
BRT_RENDERER_FACILITY_O		EQU		108t
BRT_RENDERER_FACILITY_OL		EQU		432t
BRT_RENDERER_MAX_I32		EQU		106t
BRT_RENDERER_O		EQU		103t
BRT_RENDERER_STATE_STORED		EQU		44t
BRT_RENDER_B		EQU		256t
BRT_RGB		EQU		137t
BRT_RGBA_4_4_4_4		EQU		337t
BRT_RGBA_8_8_8_8		EQU		333t
BRT_RGBX_8_8_8_8		EQU		329t
BRT_RGB_2_3_2		EQU		322t
BRT_RGB_5_6_5		EQU		320t
BRT_RGB_8_8_8		EQU		324t
BRT_RIGHT_LEFT		EQU		65t
BRT_SCREEN_DOOR_O		EQU		168t
BRT_SCREEN_DOOR_TVL		EQU		399t
BRT_SHARED_B		EQU		109t
BRT_SMOOTH_1_B		EQU		475t
BRT_SMOOTH_B		EQU		126t
BRT_SPACE_T		EQU		175t
BRT_SPECULAR_F		EQU		197t
BRT_SPECULAR_POWER_F		EQU		199t
BRT_SPECULAR_POWER_X		EQU		198t
BRT_SPECULAR_X		EQU		196t
BRT_SPOT		EQU		210t
BRT_SPOT_INNER_F		EQU		218t
BRT_SPOT_INNER_X		EQU		217t
BRT_SPOT_OUTER_F		EQU		216t
BRT_SPOT_OUTER_X		EQU		215t
BRT_SPRITE		EQU		265t
BRT_SQUARE		EQU		350t
BRT_STANDARD_BLEND		EQU		491t
BRT_STATE_RESTORE		EQU		269t
BRT_STEREO_B		EQU		59t
BRT_STEREO_T		EQU		60t
BRT_STEREO_TL		EQU		61t
BRT_STORED_STATE_O		EQU		264t
BRT_STRETCHDIBITS		EQU		409t
BRT_STRETCH_COPY_DESTINATION_TVL		EQU		365t
BRT_STRETCH_COPY_F		EQU		369t
BRT_STRETCH_COPY_SOURCE_TVL		EQU		364t
BRT_STRETCH_COPY_T		EQU		366t
BRT_STRETCH_COPY_TL		EQU		367t
BRT_STRETCH_COPY_X		EQU		368t
BRT_STRING		EQU		31t
BRT_SUBDIVIDE		EQU		381t
BRT_SUBDIVIDE_TOLERANCE_I32		EQU		382t
BRT_SURFACE		EQU		117t
BRT_SURFACE_B		EQU		253t
BRT_SURFACE_C_CUSTOM_P		EQU		202t
BRT_SURFACE_C_FILTER_P		EQU		203t
BRT_SURFACE_V_CUSTOM_P		EQU		204t
BRT_SURFACE_V_FILTER_P		EQU		205t
BRT_SYCHRONISE_I32		EQU		372t
BRT_TEXTURE_1_O		EQU		479t
BRT_TEXTURE_MEMORY_U32		EQU		92t
BRT_TEXTURE_O		EQU		164t
BRT_TEXTURE_TVL		EQU		395t
BRT_TITLE_CSTR		EQU		358t
BRT_TOKEN		EQU		4t
BRT_TOKEN_LIST		EQU		34t
BRT_TOKEN_VALUE_LIST		EQU		355t
BRT_TOP_BOTTOM		EQU		66t
BRT_TRANSFORM_B		EQU		252t
BRT_TRIANGLE		EQU		267t
BRT_TRI_FAN		EQU		466t
BRT_TRI_STRIP		EQU		465t
BRT_TWO_SIDED		EQU		174t
BRT_TYPE_T		EQU		172t
BRT_UINT_16		EQU		8t
BRT_UINT_32		EQU		10t
BRT_UINT_8		EQU		6t
BRT_UNKNOWN		EQU		171t
BRT_USES_PRIMITIVE_LIBRARY_B		EQU		435t
BRT_USE_BACKGROUND_ALPHA_B		EQU		484t
BRT_USE_CONSTANT_ALPHA_B		EQU		482t
BRT_USE_CONSTANT_FOG_B		EQU		483t
BRT_USE_NO_3D_B		EQU		279t
BRT_USE_NO_DEPTH_B		EQU		280t
BRT_USE_T		EQU		99t
BRT_V1BUCKETS		EQU		272t
BRT_V1INSERT_ARG1_P		EQU		261t
BRT_V1INSERT_ARG2_P		EQU		262t
BRT_V1INSERT_ARG3_P		EQU		263t
BRT_V1INSERT_FUNCTION_P		EQU		260t
BRT_V1MODEL		EQU		271t
BRT_V1ORDER_TABLE_P		EQU		258t
BRT_V1PRIMITIVE_HEAP_P		EQU		259t
BRT_VECTOR2_FIXED		EQU		19t
BRT_VECTOR2_FLOAT		EQU		22t
BRT_VECTOR2_INTEGER		EQU		16t
BRT_VECTOR3_FIXED		EQU		20t
BRT_VECTOR3_FLOAT		EQU		23t
BRT_VECTOR3_INTEGER		EQU		17t
BRT_VECTOR4_FIXED		EQU		21t
BRT_VECTOR4_FLOAT		EQU		24t
BRT_VECTOR4_INTEGER		EQU		18t
BRT_VERSION_U32		EQU		55t
BRT_VERTEX_DBL		EQU		464t
BRT_VERTEX_SGL		EQU		463t
BRT_VERTICAL_BLANK		EQU		274t
BRT_VIDEO_MEMORY_U32		EQU		91t
BRT_VIEW		EQU		176t
BRT_VIEW_TO_ENVIRONMENT_HINT_T		EQU		245t
BRT_VIEW_TO_ENVIRONMENT_M34_F		EQU		243t
BRT_VIEW_TO_ENVIRONMENT_M34_X		EQU		244t
BRT_VIEW_TO_SCREEN_HINT_T		EQU		240t
BRT_VIEW_TO_SCREEN_M4_F		EQU		238t
BRT_VIEW_TO_SCREEN_M4_X		EQU		239t
BRT_WIDTH_I32		EQU		69t
BRT_WIDTH_MAX_I32		EQU		71t
BRT_WIDTH_MIN_I32		EQU		70t
BRT_WIDTH_OPTIMUM_I32		EQU		391t
BRT_WIDTH_T		EQU		387t
BRT_WIDTH_TL		EQU		388t
BRT_WINDOW_BUFFER_METHOD_T		EQU		407t
BRT_WINDOW_FULLSCREEN_B		EQU		406t
BRT_WINDOW_HANDLE_H		EQU		405t
BRT_WINDOW_NAME_CSTR		EQU		403t
BRT_WINDOW_PROCEDURE_P		EQU		404t
BRT_WING		EQU		408t
BRT_WORD_BYTES_I32		EQU		77t
BRT_WRAP		EQU		307t
BRT_WRITABLE_BASE_I32		EQU		284t
BRT_WRITABLE_RANGE_I32		EQU		285t
BRT_XBGR_1_5_5_5		EQU		317t
BRT_XBGR_8_8_8_8		EQU		327t
BRT_XRGB_1_5_5_5		EQU		316t
BRT_XRGB_8_8_8_8		EQU		326t
BRT_XYZ_16_16_16		EQU		347t
BRT_XYZ_8_8_8		EQU		346t
BRT_YON_Z_F		EQU		439t
BRT_YON_Z_X		EQU		440t
BRT_YUV_16		EQU		348t
BRT_ZERO_IS_TRANSPARENT		EQU		352t
BRT_ZTRANSPARENCY_B		EQU		451t
BRT_Z_WRITE_ALWAYS		EQU		448t
BRT_Z_WRITE_CONDITIONAL		EQU		450t
BRT_Z_WRITE_CONDITIONAL_NO_Z		EQU		449t
BRT_Z_WRITE_NEVER		EQU		447t
BRT_Z_WRITE_T		EQU		445t
BRT_Z_WRITE_TL		EQU		446t
_BRT_MAX		EQU		134217727t

br_token		TYPEDEF		SDWORD

; End of file D:\CrocPC\croc\source\brender\fw\pretok.h
@t_31		TYPEDEF		PTR 
@t_32		TYPEDEF		PTR br_object
@t_33		TYPEDEF		PTR 
@t_34		TYPEDEF		PTR br_vector2_i
@t_35		TYPEDEF		PTR br_vector3_i
@t_36		TYPEDEF		PTR br_vector4_i
@t_37		TYPEDEF		PTR br_vector2_x
@t_38		TYPEDEF		PTR br_vector3_x
@t_39		TYPEDEF		PTR br_vector4_x
@t_40		TYPEDEF		PTR br_vector2_f
@t_41		TYPEDEF		PTR br_vector3_f
@t_42		TYPEDEF		PTR br_vector4_f
@t_43		TYPEDEF		PTR br_matrix23_x
@t_44		TYPEDEF		PTR br_matrix34_x
@t_45		TYPEDEF		PTR br_matrix4_x
@t_46		TYPEDEF		PTR br_matrix23_f
@t_47		TYPEDEF		PTR br_matrix34_f
@t_48		TYPEDEF		PTR br_matrix4_f
@t_49		TYPEDEF		PTR SBYTE
@t_50		TYPEDEF		PTR SBYTE
@t_51		TYPEDEF		PTR PTR br_object
@t_52		TYPEDEF		PTR br_token
@t_53		TYPEDEF		PTR PTR 
@t_54		TYPEDEF		PTR br_token_value
br_value_tag		UNION
u32		br_uint_32		?
_b		br_boolean		?
p		@t_31		?
t		br_token		?
i8		br_int_8		?
u8		br_uint_8		?
i16		br_int_16		?
u16		br_uint_16		?
i32		br_int_32		?
x		br_fixed_ls		?
f		br_float		?
s		br_scalar		?
_a		br_angle		?
rgb		br_colour		?
o		@t_32		?
h		@t_33		?
v2_i		@t_34		?
v3_i		@t_35		?
v4_i		@t_36		?
v2_x		@t_37		?
v3_x		@t_38		?
v4_x		@t_39		?
v2_f		@t_40		?
v3_f		@t_41		?
v4_f		@t_42		?
m23_x		@t_43		?
m34_x		@t_44		?
m4_x		@t_45		?
m23_f		@t_46		?
m34_f		@t_47		?
m4_f		@t_48		?
_str		@t_49		?
cstr		@t_50		?
ol		@t_51		?
tl		@t_52		?
pl		@t_53		?
tvl		@t_54		?
br_value_tag		ENDS

br_value		TYPEDEF		br_value_tag

br_token_value		STRUCT 4t
t		br_token		?
v		br_value		<>
br_token_value		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\token.h
; Begin of file D:\CrocPC\croc\source\brender\inc\render.h
BR_STATE_SURFACE		EQU		1t
BR_STATE_MATRIX		EQU		2t
BR_STATE_ENABLE		EQU		4t
BR_STATE_LIGHT		EQU		8t
BR_STATE_CLIP		EQU		16t
BR_STATE_BOUNDS		EQU		32t
BR_STATE_CULL		EQU		64t
BR_STATE_STATS		EQU		128t
BR_STATE_CACHE		EQU		256t
BR_STATE_OUTPUT		EQU		1073741824t
BR_STATE_PRIMITIVE		EQU		-2147483648t
BR_STATE_ALL		EQU		-1t

; End of file D:\CrocPC\croc\source\brender\inc\render.h
; Begin of file D:\CrocPC\croc\source\brender\inc\pool.h
BR_POOL_DEBUG		EQU		0t
BR_POOL_ALIGN		EQU		7t
@t_55		TYPEDEF		PTR br_pool_block
br_pool_block		STRUCT 4t
next		@t_55		?
br_pool_block		ENDS

@t_56		TYPEDEF		PTR br_pool_block
br_pool		STRUCT 4t
free		@t_56		?
block_size		br_uint_32		?
chunk_size		br_uint_32		?
mem_type		SDWORD		?
br_pool		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\pool.h
; Begin of file D:\CrocPC\croc\source\brender\inc\custsupt.h
OUTCODE_LEFT		EQU		000000001h
OUTCODE_RIGHT		EQU		000000002h
OUTCODE_TOP		EQU		000000004h
OUTCODE_BOTTOM		EQU		000000008h
OUTCODE_HITHER		EQU		000000010h
OUTCODE_YON		EQU		000000020h
OUTCODE_USER		EQU		000000040h
OUTCODE_USER_ALL		EQU		000000fc0h
OUTCODES_ALL		EQU		000000fffh
OUTCODE_N_LEFT		EQU		000010000h
OUTCODE_N_RIGHT		EQU		000020000h
OUTCODE_N_TOP		EQU		000040000h
OUTCODE_N_BOTTOM		EQU		000080000h
OUTCODE_N_HITHER		EQU		000100000h
OUTCODE_N_YON		EQU		000200000h
OUTCODE_N_USER		EQU		000400000h
OUTCODE_N_USER_ALL		EQU		00fc00000h
OUTCODES_NOT		EQU		00fff0000h
; End of file D:\CrocPC\croc\source\brender\inc\custsupt.h
; Begin of file D:\CrocPC\croc\source\brender\inc\order.h
BR_ORDER_TABLE_LEAVE_BOUNDS		EQU		0t
BR_ORDER_TABLE_NEW_BOUNDS		EQU		1t
BR_ORDER_TABLE_INIT_BOUNDS		EQU		2t
BR_ORDER_TABLE_CONVEX		EQU		4t
BR_ORDER_TABLE_SORT_NEAR		EQU		8t
BR_ORDER_TABLE_SORT_FAR		EQU		16t
BR_ORDER_TABLE_SORT_CENTRE		EQU		32t

BR_SORT_AVERAGE		EQU		1t
BR_SORT_MIN		EQU		2t
BR_SORT_MAX		EQU		3t
BR_SORT_FIRST_VERTEX		EQU		4t

BR_PRIMITIVE_POINT		EQU		209t
BR_PRIMITIVE_LINE		EQU		266t
BR_PRIMITIVE_TRIANGLE		EQU		267t

@t_57		TYPEDEF		PTR br_primitive
@t_58		TYPEDEF		PTR 
@t_59		TYPEDEF		PTR 
br_primitive		STRUCT 4t
next		@t_57		?
_type		br_uint_32		?
stored		@t_58		?
v		@t_59		3t DUP (?)
br_primitive		ENDS

@t_60		TYPEDEF		PTR PTR br_primitive
@t_61		TYPEDEF		PTR br_order_table
br_order_table		STRUCT 4t
table		@t_60		?
_size		br_uint_16		?
next		@t_61		?
min_z		br_scalar		?
max_z		br_scalar		?
sort_z		br_scalar		?
scale		br_scalar		?
flags		br_uint_32		?
_type		br_uint_16		?
visits		br_uint_16		?
br_order_table		ENDS

@t_62		TYPEDEF		PTR br_uint_8
@t_63		TYPEDEF		PTR br_uint_8
br_primitive_heap		STRUCT 4t
base		@t_62		?
_size		br_size_t		?
current		@t_63		?
br_primitive_heap		ENDS

@tag_16		STRUCT 4t
screenX		br_scalar		?
screenY		br_scalar		?
distanceZ		br_scalar		?
@tag_16		ENDS

ot_vertex		TYPEDEF		@tag_16

; End of file D:\CrocPC\croc\source\brender\inc\order.h
; Begin of file D:\CrocPC\croc\source\brender\inc\assocarr.h
BR_ASSOCIATIVE_ARRAY_BLOCK_SIZE		EQU		10t
@t_64		TYPEDEF		PTR br_token_value
br_associative_array_tag		STRUCT 4t
tv		@t_64		?
num_elements		br_uint_16		?
max_elements		br_uint_16		?
br_associative_array_tag		ENDS

br_associative_array		TYPEDEF		br_associative_array_tag

; End of file D:\CrocPC\croc\source\brender\inc\assocarr.h
__BR_V1DB__		EQU		1t
; Begin of file D:\CrocPC\croc\source\brender\inc\brv1db.h
; Begin of file D:\CrocPC\croc\source\brender\inc\material.h
@t_65		TYPEDEF		PTR SBYTE
@t_66		TYPEDEF		PTR br_pixelmap
@t_67		TYPEDEF		PTR br_pixelmap
@t_68		TYPEDEF		PTR br_pixelmap
@t_69		TYPEDEF		PTR br_pixelmap
@t_70		TYPEDEF		PTR br_pixelmap
@t_71		TYPEDEF		PTR br_token_value
@t_72		TYPEDEF		PTR br_token_value
@t_73		TYPEDEF		PTR br_pixelmap
@t_74		TYPEDEF		PTR 
@t_75		TYPEDEF		PTR 
br_material		STRUCT 4t
_reserved		br_uint_32		?
identifier		@t_65		?
colour		br_colour		?
opacity		br_uint_8		?
ka		br_ufraction		?
kd		br_ufraction		?
ks		br_ufraction		?
power		br_scalar		?
ambient_colour		br_colour		?
flags		br_uint_32		?
map_transform		br_matrix23		<>
index_base		br_uint_8		?
index_range		br_uint_8		?
colour_map		@t_66		?
screendoor		@t_67		?
index_shade		@t_68		?
index_blend		@t_69		?
index_fog		@t_70		?
extra_surf		@t_71		?
extra_prim		@t_72		?
fog_min		br_scalar		?
fog_max		br_scalar		?
fog_colour		br_colour		?
flags_ext		br_uint_32		?
colour_map_1		@t_73		?
map_transform_1		br_matrix23		<>
mip_modifier		br_int_8		?
min_mip		br_uint_8		?
max_mip		br_uint_8		?
alpha_mode		br_uint_8		?
zbuffer_mode		br_uint_8		?
zbuffer_compare		br_uint_8		?
subdivide_tolerance		br_int_32		?
user		@t_74		?
stored		@t_75		?
br_material		ENDS

BR_MATF_LIGHT		EQU		1t
BR_MATF_PRELIT		EQU		2t
BR_MATF_SMOOTH		EQU		4t
BR_MATF_ENVIRONMENT_I		EQU		8t
BR_MATF_ENVIRONMENT_L		EQU		16t
BR_MATF_PERSPECTIVE		EQU		32t
BR_MATF_DECAL		EQU		64t
BR_MATF_I_FROM_U		EQU		128t
BR_MATF_I_FROM_V		EQU		256t
BR_MATF_U_FROM_I		EQU		512t
BR_MATF_V_FROM_I		EQU		1024t
BR_MATF_ALWAYS_VISIBLE		EQU		2048t
BR_MATF_TWO_SIDED		EQU		4096t
BR_MATF_FORCE_FRONT		EQU		8192t
BR_MATF_DITHER		EQU		16384t
BR_MATF_MAP_ANTIALIASING		EQU		65536t
BR_MATF_MAP_INTERPOLATION		EQU		131072t
BR_MATF_MIP_INTERPOLATION		EQU		262144t
BR_MATF_FOG_LOCAL		EQU		524288t
BR_MATF_SUBDIVIDE		EQU		1048576t
BR_MATF_ZTRANSPARENCY		EQU		2097152t
BR_MATF_QUAD_MAPPING		EQU		4194304t
BR_MATF_FORCE_BACK		EQU		8388608t
BR_MATF_NO_RENDER		EQU		16777216t
BR_MATF_NO_SCALABILITY		EQU		33554432t
BR_MATF_MAP		EQU		67108864t
BR_MATF_FLAT		EQU		134217728t

BR_MATF_MAP_COLOUR		EQU		0t
BR_MATF_BLEND		EQU		0t
BR_MATFX_TEXTURE0_MIRRORED_U		EQU		1t
BR_MATFX_TEXTURE0_CLAMPED_U		EQU		2t
BR_MATFX_TEXTURE0_MIRRORED_V		EQU		4t
BR_MATFX_TEXTURE0_CLAMPED_V		EQU		8t
BR_MATFX_TEXTURE1_MIRRORED_U		EQU		16t
BR_MATFX_TEXTURE1_CLAMPED_U		EQU		32t
BR_MATFX_TEXTURE1_MIRRORED_V		EQU		64t
BR_MATFX_TEXTURE1_CLAMPED_V		EQU		128t
BR_MATFX_EDGE_ANTIALIASING		EQU		256t
BR_MATFX_DOUBLE_TEXTURE		EQU		512t
BR_MATFX_CONSTANT_FOG		EQU		1024t
BR_MATFX_CONSTANT_ALPHA		EQU		2048t
BR_MATFX_BACKGROUND_ALPHA		EQU		4096t
BR_MATFX_MIP_MOD_ABSOLUTE		EQU		8192t
BR_MATFX_SMOOTH_1		EQU		16384t

BR_MATB_NONE		EQU		0t
BR_MATB_INTENSITY_REDUCTION		EQU		1t
BR_MATB_COMPOSITING_WITH_REDUCTION		EQU		2t
BR_MATB_STANDARD_BLEND		EQU		3t
BR_MATB_COMPOSITING		EQU		4t
BR_MATB_COMPOSITING_WITH_INCREASE		EQU		5t

BR_MATZM_STANDARD		EQU		0t
BR_MATZM_ALWAYS_WRITE		EQU		1t
BR_MATZM_DONT_WRITE		EQU		2t
BR_MATZM_OFF		EQU		3t

BR_MATZC_LT		EQU		0t
BR_MATZC_GT		EQU		1t
BR_MATZC_LE		EQU		2t
BR_MATZC_GE		EQU		3t
BR_MATZC_EQ		EQU		4t
BR_MATZC_NE		EQU		5t

BR_MATU_MAP_TRANSFORM_0		EQU		1t
BR_MATU_RENDERING		EQU		2t
BR_MATU_LIGHTING		EQU		4t
BR_MATU_COLOURMAP		EQU		8t
BR_MATU_SCREENDOOR		EQU		16t
BR_MATU_EXTRA_SURF		EQU		32t
BR_MATU_EXTRA_PRIM		EQU		64t
BR_MATU_MAP_TRANSFORM_1		EQU		128t
BR_MATU_ALL		EQU		32767t

BR_MAPU_ALL		EQU		4095t
BR_MAPU_SHARED		EQU		32768t

BR_TABU_ALL		EQU		32767t

; End of file D:\CrocPC\croc\source\brender\inc\material.h
; Begin of file D:\CrocPC\croc\source\brender\inc\model.h
br_vertex		STRUCT 4t
p		br_vector3		<>
map		br_vector2		<>
index		br_uint_8		?
red		br_uint_8		?
grn		br_uint_8		?
blu		br_uint_8		?
_pad0		br_uint_16		?
n		br_fvector3		<>
br_vertex		ENDS

@t_76		TYPEDEF		PTR br_material
br_face		STRUCT 4t
vertices		br_uint_16		3t DUP (?)
smoothing		br_uint_16		?
material		@t_76		?
index		br_uint_8		?
red		br_uint_8		?
grn		br_uint_8		?
blu		br_uint_8		?
flags		br_uint_8		?
_pad0		br_uint_8		?
_pad1		br_uint_32		?
n		br_fvector3		<>
d		br_scalar		?
br_face		ENDS

BR_FACEF_COPLANAR_0		EQU		1t
BR_FACEF_COPLANAR_1		EQU		2t
BR_FACEF_COPLANAR_2		EQU		4t
BR_FACEF_COPLANAR_3		EQU		8t
BR_FACEF_QUAD_MASK		EQU		112t
BR_FACEF_QUAD_012		EQU		0t
BR_FACEF_QUAD_123		EQU		16t
BR_FACEF_QUAD_230		EQU		32t
BR_FACEF_QUAD_301		EQU		48t
BR_FACEF_QUAD_032		EQU		64t
BR_FACEF_QUAD_103		EQU		80t
BR_FACEF_QUAD_210		EQU		96t
BR_FACEF_QUAD_321		EQU		112t
BR_FACEF_QUAD_SHIFT		EQU		4t

@t_77		TYPEDEF		PTR br_primitive_list
@t_78		TYPEDEF		PTR 
@t_79		TYPEDEF		PTR 
br_primitive_list		STRUCT 4t
next		@t_77		?
prim_type		br_uint_32		?
nprims		br_uint_16		?
nspares		br_uint_16		?
prim		@t_78		?
spare		@t_79		?
br_primitive_list		ENDS

BR_PRIM_VERTEX_SGL		EQU		1t
BR_PRIM_VERTEX_DBL		EQU		2t
BR_PRIM_POINT		EQU		3t
BR_PRIM_LINE		EQU		4t
BR_PRIM_TRIANGLE		EQU		5t
BR_PRIM_QUAD		EQU		6t
BR_PRIM_TRI_STRIP		EQU		7t
BR_PRIM_TRI_FAN		EQU		8t
BR_PRIM_QUAD_STRIP		EQU		9t

br_vertex_single		STRUCT 4t
p		br_vector3		<>
map		br_vector2		<>
alpha		br_uint_8		?
red		br_uint_8		?
grn		br_uint_8		?
blu		br_uint_8		?
_pad0		br_uint_16		?
n		br_fvector3		<>
br_vertex_single		ENDS

br_vertex_double		STRUCT 4t
p		br_vector3		<>
map0		br_vector2		<>
alpha0		br_uint_8		?
red0		br_uint_8		?
grn0		br_uint_8		?
blu0		br_uint_8		?
map1		br_vector2		<>
alpha1		br_uint_8		?
red1		br_uint_8		?
grn1		br_uint_8		?
blu1		br_uint_8		?
n		br_fvector3		<>
br_vertex_double		ENDS

@t_80		TYPEDEF		PTR br_material
br_point_prim		STRUCT 4t
vertices		br_uint_16		1t DUP (?)
material		@t_80		?
br_point_prim		ENDS

@t_81		TYPEDEF		PTR br_material
br_line		STRUCT 4t
vertices		br_uint_16		2t DUP (?)
material		@t_81		?
alpha0		br_uint_8		?
red0		br_uint_8		?
grn0		br_uint_8		?
blu0		br_uint_8		?
alpha1		br_uint_8		?
red1		br_uint_8		?
grn1		br_uint_8		?
blu1		br_uint_8		?
br_line		ENDS

@t_82		TYPEDEF		PTR br_material
br_triangle		STRUCT 4t
vertices		br_uint_16		3t DUP (?)
smoothing		br_uint_16		?
material		@t_82		?
alpha0		br_uint_8		?
red0		br_uint_8		?
grn0		br_uint_8		?
blu0		br_uint_8		?
alpha1		br_uint_8		?
red1		br_uint_8		?
grn1		br_uint_8		?
blu1		br_uint_8		?
flags		br_uint_8		?
n		br_fvector3		<>
d		br_scalar		?
br_triangle		ENDS

@t_83		TYPEDEF		PTR br_material
br_quad		STRUCT 4t
vertices		br_uint_16		4t DUP (?)
smoothing		br_uint_16		?
material		@t_83		?
alpha0		br_uint_8		?
red0		br_uint_8		?
grn0		br_uint_8		?
blu0		br_uint_8		?
alpha1		br_uint_8		?
red1		br_uint_8		?
grn1		br_uint_8		?
blu1		br_uint_8		?
flags		br_uint_8		?
n		br_fvector3		<>
d		br_scalar		?
br_quad		ENDS

br_strip_face_data		STRUCT 4t
smoothing		br_uint_16		?
alpha0		br_uint_8		?
red0		br_uint_8		?
grn0		br_uint_8		?
blu0		br_uint_8		?
alpha1		br_uint_8		?
red1		br_uint_8		?
grn1		br_uint_8		?
blu1		br_uint_8		?
flags		br_uint_8		?
n		br_fvector3		<>
d		br_scalar		?
br_strip_face_data		ENDS

@t_84		TYPEDEF		PTR br_uint_16
@t_85		TYPEDEF		PTR br_material
@t_86		TYPEDEF		PTR br_strip_face_data
br_tri_strip		STRUCT 4t
nvertices		br_uint_16		?
vertices		@t_84		?
material		@t_85		?
face_data		@t_86		?
br_tri_strip		ENDS

br_tri_fan		TYPEDEF		br_tri_strip

br_quad_strip		TYPEDEF		br_tri_strip

@proto_21		TYPEDEF		PROTO STDCALL :PTR br_actor, :PTR br_model, :PTR br_material, :PTR , :br_uint_8, :SDWORD
br_model_custom_cbfn		TYPEDEF		@proto_21

@t_87		TYPEDEF		PTR SBYTE
@t_88		TYPEDEF		PTR br_vertex
@t_89		TYPEDEF		PTR br_face
@t_90		TYPEDEF		PTR br_model_custom_cbfn
@t_91		TYPEDEF		PTR 
@t_92		TYPEDEF		PTR 
@t_93		TYPEDEF		PTR 
@t_94		TYPEDEF		PTR br_primitive_list
br_model		STRUCT 4t
_reserved		br_uint_32		?
identifier		@t_87		?
vertices		@t_88		?
faces		@t_89		?
nvertices		br_uint_16		?
nfaces		br_uint_16		?
pivot		br_vector3		<>
flags		br_uint_16		?
custom		@t_90		?
user		@t_91		?
crocFlags		SDWORD		?
crease_angle		br_angle		?
radius		br_scalar		?
bounds		br_bounds		<>
prepared		@t_92		?
stored		@t_93		?
nprimitive_lists		br_uint_16		?
primitive_list		@t_94		?
br_model		ENDS

BR_MODF_DONT_WELD		EQU		1t
BR_MODF_KEEP_ORIGINAL		EQU		2t
BR_MODF_GENERATE_TAGS		EQU		4t
BR_MODF_QUICK_UPDATE		EQU		16t
BR_MODF_CUSTOM		EQU		32t
BR_MODF_PREPREPARED		EQU		64t
BR_MODF_UPDATEABLE		EQU		128t
BR_MODF_CREASE		EQU		256t
BR_MODF_CUSTOM_NORMALS		EQU		512t
BR_MODF_CUSTOM_BOUNDS		EQU		1024t
BR_MODF_FACES_ONLY		EQU		2048t
_BR_MODF_RESERVED		EQU		32768t

BR_MODU_VERTEX_POSITIONS		EQU		1t
BR_MODU_VERTEX_COLOURS		EQU		2t
BR_MODU_VERTEX_MAPPING		EQU		4t
BR_MODU_VERTEX_NORMALS		EQU		8t
BR_MODU_PRIMITIVE_MATERIALS		EQU		16t
BR_MODU_PRIMITIVE_COLOURS		EQU		32t
BR_MODU_VERTICES		EQU		64t
BR_MODU_FACES		EQU		128t
BR_MODU_PIVOT		EQU		256t
BR_MODU_ALL		EQU		32767t

BR_MODCROCF_SPRITE		EQU		1t

BR_APPLYMAP_PLANE		EQU		0t
BR_APPLYMAP_SPHERE		EQU		1t
BR_APPLYMAP_CYLINDER		EQU		2t
BR_APPLYMAP_DISC		EQU		3t
BR_APPLYMAP_NONE		EQU		4t

BR_FITMAP_PLUS_X		EQU		0t
BR_FITMAP_PLUS_Y		EQU		1t
BR_FITMAP_PLUS_Z		EQU		2t
BR_FITMAP_MINUS_X		EQU		3t
BR_FITMAP_MINUS_Y		EQU		4t
BR_FITMAP_MINUS_Z		EQU		5t

; End of file D:\CrocPC\croc\source\brender\inc\model.h
; Begin of file D:\CrocPC\croc\source\brender\inc\actor.h
BR_ACTOR_NONE		EQU		0t
BR_ACTOR_MODEL		EQU		1t
BR_ACTOR_LIGHT		EQU		2t
BR_ACTOR_CAMERA		EQU		3t
_BR_ACTOR_RESERVED		EQU		4t
BR_ACTOR_BOUNDS		EQU		5t
BR_ACTOR_BOUNDS_CORRECT		EQU		6t
BR_ACTOR_CLIP_PLANE		EQU		7t
BR_ACTOR_HORIZON_PLANE		EQU		8t
BR_ACTOR_MAX		EQU		9t

BR_RSTYLE_DEFAULT		EQU		0t
BR_RSTYLE_NONE		EQU		1t
BR_RSTYLE_POINTS		EQU		2t
BR_RSTYLE_EDGES		EQU		3t
BR_RSTYLE_FACES		EQU		4t
BR_RSTYLE_BOUNDING_POINTS		EQU		5t
BR_RSTYLE_BOUNDING_EDGES		EQU		6t
BR_RSTYLE_BOUNDING_FACES		EQU		7t
BR_RSTYLE_ANTIALIASED_LINES		EQU		8t
BR_RSTYLE_ANTIALIASED_FACES		EQU		9t
BR_RSTYLE_MAX		EQU		10t

@t_95		TYPEDEF		PTR br_actor
@t_96		TYPEDEF		PTR PTR br_actor
@t_97		TYPEDEF		PTR br_actor
@t_98		TYPEDEF		PTR br_actor
@t_99		TYPEDEF		PTR SBYTE
@t_100		TYPEDEF		PTR br_model
@t_101		TYPEDEF		PTR br_material
@t_102		TYPEDEF		PTR 
@t_103		TYPEDEF		PTR 
@t_104		TYPEDEF		PTR 
br_actor		STRUCT 4t
next		@t_95		?
prev		@t_96		?
children		@t_97		?
parent		@t_98		?
depth		br_uint_16		?
_type		br_uint_8		?
identifier		@t_99		?
model		@t_100		?
material		@t_101		?
render_style		br_uint_8		?
render_data		@t_102		?
t		br_transform		<>
type_data		@t_103		?
user		@t_104		?
br_actor		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\actor.h
; Begin of file D:\CrocPC\croc\source\brender\inc\camera.h
BR_CAMERA_PARALLEL		EQU		0t
BR_CAMERA_PERSPECTIVE_FOV		EQU		1t
BR_CAMERA_PERSPECTIVE_WHD		EQU		2t
BR_CAMERA_PARALLEL_OLD		EQU		3t
BR_CAMERA_PERSPECTIVE_FOV_OLD		EQU		4t

@t_105		TYPEDEF		PTR SBYTE
@t_106		TYPEDEF		PTR 
br_camera		STRUCT 4t
identifier		@t_105		?
_type		br_uint_8		?
field_of_view		br_angle		?
hither_z		br_scalar		?
yon_z		br_scalar		?
aspect		br_scalar		?
_width		br_scalar		?
height		br_scalar		?
distance		br_scalar		?
user		@t_106		?
br_camera		ENDS

; End of file D:\CrocPC\croc\source\brender\inc\camera.h
; Begin of file D:\CrocPC\croc\source\brender\inc\light.h
BR_LIGHT_TYPE		EQU		3t
BR_LIGHT_POINT		EQU		0t
BR_LIGHT_DIRECT		EQU		1t
BR_LIGHT_SPOT		EQU		2t
BR_LIGHT_VIEW		EQU		4t

@t_107		TYPEDEF		PTR SBYTE
@t_108		TYPEDEF		PTR 
br_light		STRUCT 4t
identifier		@t_107		?
_type		br_uint_8		?
colour		br_colour		?
attenuation_c		br_scalar		?
attenuation_l		br_scalar		?
attenuation_q		br_scalar		?
cone_outer		br_angle		?
cone_inner		br_angle		?
user		@t_108		?
br_light		ENDS

EXTERNDEF		C globalAmbientRed:br_scalar

EXTERNDEF		C globalAmbientGreen:br_scalar

EXTERNDEF		C globalAmbientBlue:br_scalar

EXTERNDEF		C globalAmbient:br_scalar

; End of file D:\CrocPC\croc\source\brender\inc\light.h
; Begin of file D:\CrocPC\croc\source\brender\fmt\fmt.h
@t_109		TYPEDEF		PTR PTR br_actor
@t_110		TYPEDEF		PTR PTR br_model
@t_111		TYPEDEF		PTR PTR br_light
@t_112		TYPEDEF		PTR PTR br_camera
@t_113		TYPEDEF		PTR PTR br_vector4
@t_114		TYPEDEF		PTR PTR br_bounds3
@t_115		TYPEDEF		PTR PTR br_bounds3
@t_116		TYPEDEF		PTR PTR br_material
@t_117		TYPEDEF		PTR PTR br_pixelmap
br_fmt_results		STRUCT 4t
actors		@t_109		?
models		@t_110		?
lights		@t_111		?
cameras		@t_112		?
clip_planes		@t_113		?
bounds		@t_114		?
bounds_correct		@t_115		?
materials		@t_116		?
pixelmaps		@t_117		?
nactors		br_uint_32		?
nmodels		br_uint_32		?
nlights		br_uint_32		?
ncameras		br_uint_32		?
nclip_planes		br_uint_32		?
nbounds		br_uint_32		?
nbounds_correct		br_uint_32		?
nmaterials		br_uint_32		?
npixelmaps		br_uint_32		?
br_fmt_results		ENDS

br_fmt_options		STRUCT 4t
convert_flags		br_uint_32		?
br_fmt_options		ENDS

BR_FMT_ACTORS		EQU		1t
BR_FMT_MODELS		EQU		2t
BR_FMT_LIGHTS		EQU		4t
BR_FMT_CAMERAS		EQU		8t
BR_FMT_CLIP_PLANES		EQU		16t
BR_FMT_BOUNDS		EQU		32t
BR_FMT_BOUNDS_CORRECT		EQU		64t
BR_FMT_MATERIALS		EQU		128t
BR_FMT_PIXELMAPS		EQU		256t
BR_FMT_CONVERT_ALL		EQU		32767t

@proto_22		TYPEDEF		PROTO STDCALL :PTR SBYTE
br_fmt_report_cbfn		TYPEDEF		@proto_22

@t_118		TYPEDEF		PTR br_fmt_report_cbfn
br_3ds_options		STRUCT 4t
flags		br_uint_32		?
hither		br_scalar		?
yon		br_scalar		?
scale		br_scalar		?
report		@t_118		?
br_3ds_options		ENDS

BR_3DS_FLAT_HIERARCHY		EQU		1t
BR_3DS_SET_PERSPECTIVE		EQU		2t
BR_3DS_CORRECT_AXES		EQU		4t
BR_3DS_APPLY_MESH_MATRIX		EQU		8t
BR_3DS_APPLY_PIVOT		EQU		16t
BR_3DS_DUMMY_LIGHTS		EQU		32t
BR_3DS_DUMMY_CAMERAS		EQU		64t
BR_3DS_SET_HITHER		EQU		128t
BR_3DS_SET_YON		EQU		256t
BR_3DS_APPLY_SCALE		EQU		512t
BR_3DS_KEEP_REPEATED_MODELS		EQU		1024t

; Begin of file D:\CrocPC\croc\source\brender\fmt\fmt_p.h
@proto_23		TYPEDEF		PROTO C :PTR SBYTE, :PTR br_fmt_options, :PTR br_3ds_options
BrFmt3DSLoad		PROTO		@proto_23

@proto_24		TYPEDEF		PROTO C :PTR SBYTE, :PTR PTR br_model, :br_uint_16
BrFmtASCLoad		PROTO		@proto_24

@proto_25		TYPEDEF		PROTO C :PTR SBYTE
BrFmtNFFLoad		PROTO		@proto_25

@proto_26		TYPEDEF		PROTO C :PTR SBYTE, :PTR PTR br_material, :br_uint_16
BrFmtScriptMaterialLoadMany		PROTO		@proto_26

@proto_27		TYPEDEF		PROTO C :PTR SBYTE
BrFmtScriptMaterialLoad		PROTO		@proto_27

@proto_28		TYPEDEF		PROTO C :PTR SBYTE, :PTR PTR br_material, :br_uint_16
BrFmtScriptMaterialSaveMany		PROTO		@proto_28

@proto_29		TYPEDEF		PROTO C :PTR SBYTE, :PTR br_material
BrFmtScriptMaterialSave		PROTO		@proto_29

@proto_30		TYPEDEF		PROTO C :PTR SBYTE, :br_uint_32
BrFmtBMPLoad		PROTO		@proto_30

@proto_31		TYPEDEF		PROTO C :PTR SBYTE, :br_uint_32
BrFmtTGALoad		PROTO		@proto_31

@proto_32		TYPEDEF		PROTO C :PTR SBYTE, :br_uint_32
BrFmtGIFLoad		PROTO		@proto_32

@proto_33		TYPEDEF		PROTO C :PTR SBYTE, :br_uint_32
BrFmtIFFLoad		PROTO		@proto_33

; End of file D:\CrocPC\croc\source\brender\fmt\fmt_p.h
; End of file D:\CrocPC\croc\source\brender\fmt\fmt.h
; End of file D:\CrocPC\croc\source\brender\inc\brv1db.h
; Begin of file D:\CrocPC\croc\source\brender\inc\compend.h
; End of file D:\CrocPC\croc\source\brender\inc\compend.h
; Begin of file D:\CrocPC\croc\source\brender\inc\assoca_p.h
; End of file D:\CrocPC\croc\source\brender\inc\assoca_p.h
; End of file D:\CrocPC\croc\source\brender\inc\brender.h
host_info		STRUCT 4t
_size		br_uint_32		?
identifier		SBYTE		40t DUP (?)
capabilities		br_uint_32		?
processor_family		br_token		?
processor_type		br_token		?
host_info		ENDS

HOST_CAPS_REAL_MEMORY		EQU		000000001h
HOST_CAPS_REAL_INT_CALL		EQU		000000002h
HOST_CAPS_REAL_INT_HOOK		EQU		000000004h
HOST_CAPS_PROTECTED_INT_CALL		EQU		000000008h
HOST_CAPS_PROTECTED_INT_HOOK		EQU		000000010h
HOST_CAPS_ALLOC_SELECTORS		EQU		000000020h
HOST_CAPS_PHYSICAL_MAP		EQU		000000040h
HOST_CAPS_EXCEPTION_HOOK		EQU		000000080h
HOST_CAPS_BASE_SELECTORS_WRITE		EQU		000000100h
HOST_CAPS_PORTS		EQU		000000200h
HOST_CAPS_MMX		EQU		000000400h
HOST_CAPS_FPU		EQU		000000800h
HOST_CAPS_CMOV		EQU		000001000h
host_real_memory		STRUCT 4t
pm_off		br_uint_32		?
pm_seg		br_uint_16		?
_reserved		br_uint_16		?
rm_off		br_uint_16		?
rm_seg		br_uint_16		?
host_real_memory		ENDS

@tag_36		STRUCT 4t
_edi		br_uint_32		?
_esi		br_uint_32		?
_ebp		br_uint_32		?
_res		br_uint_32		?
_ebx		br_uint_32		?
_edx		br_uint_32		?
_ecx		br_uint_32		?
_eax		br_uint_32		?
flags		br_uint_16		?
_es		br_uint_16		?
_ds		br_uint_16		?
_fs		br_uint_16		?
_gs		br_uint_16		?
_ip		br_uint_16		?
_cs		br_uint_16		?
_sp		br_uint_16		?
_ss		br_uint_16		?
@tag_36		ENDS

@tag_37		STRUCT 4t
_di		br_uint_16		?
_pad0		br_uint_16		?
_si		br_uint_16		?
_pad1		br_uint_16		?
_bp		br_uint_16		?
_pad2		br_uint_16		?
_res		br_uint_16		?
_pad3		br_uint_16		?
_bx		br_uint_16		?
_pad4		br_uint_16		?
_dx		br_uint_16		?
_pad5		br_uint_16		?
_cx		br_uint_16		?
_pad6		br_uint_16		?
_ax		br_uint_16		?
_pad7		br_uint_16		?
flags		br_uint_16		?
_es		br_uint_16		?
_ds		br_uint_16		?
_fs		br_uint_16		?
_gs		br_uint_16		?
_ip		br_uint_16		?
_cs		br_uint_16		?
_sp		br_uint_16		?
_ss		br_uint_16		?
@tag_37		ENDS

@tag_38		STRUCT 4t
_pad0		br_uint_32		4t DUP (?)
_bl		br_uint_8		?
_bh		br_uint_8		?
_pad1		br_uint_8		?
_pad2		br_uint_8		?
_dl		br_uint_8		?
_dh		br_uint_8		?
_pad3		br_uint_8		?
_pad4		br_uint_8		?
_cl		br_uint_8		?
_ch		br_uint_8		?
_pad5		br_uint_8		?
_pad6		br_uint_8		?
_al		br_uint_8		?
_ah		br_uint_8		?
_pad7		br_uint_8		?
_pad8		br_uint_8		?
@tag_38		ENDS

host_regs		UNION
x		@tag_36		<>
w		@tag_37		<>
h		@tag_38		<>
host_regs		ENDS

HOST_FLAG_CARRY		EQU		1t
HOST_FLAG_PARITY		EQU		4t
HOST_FLAG_AUX_CARRY		EQU		16t
HOST_FLAG_ZERO		EQU		64t
HOST_FLAG_SIGN		EQU		128t

host_interrupt_hook		STRUCT 4t
vector		br_uint_8		?
active		br_boolean		?
old_offset		br_uint_32		?
old_sel		br_uint_16		?
host_interrupt_hook		ENDS

host_exception_hook		STRUCT 4t
exception		br_uint_8		?
active		br_boolean		?
old_offset		br_uint_32		?
old_sel		br_uint_16		?
scratch		br_uint_8		256t DUP (?)
host_exception_hook		ENDS

HOST_CS_NONE		EQU		0t
HOST_CS_DRIVERS		EQU		1t
HOST_CS_OUTPUT_TYPE		EQU		2t
HOST_CS_SEARCH_PATH		EQU		3t

; Begin of file host_p.h
; End of file host_p.h
; End of file host.h
