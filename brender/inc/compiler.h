/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: compiler.h 2.18 1997/02/06 12:34:54 Johng Exp $
 * $Locker: $
 *
 * Misc host compiler configuration (types & special declarators etc.)
 */
#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <stdint.h>
#include <stddef.h>

/*
 * Fixed bitsize integers
 */

typedef int64_t br_int_64;
typedef uint64_t br_uint_64;

typedef int32_t br_int_32;
typedef uint32_t br_uint_32;

typedef int16_t br_int_16;
typedef uint16_t br_uint_16;

typedef int8_t br_int_8;
typedef uint8_t br_uint_8;

/*
 * Generic size type (in case target environment does not have size_t)
 */
typedef size_t br_size_t;
#if defined(_MSC_VER)
#	include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif

typedef intptr_t br_intptr_t;
typedef uintptr_t br_uintptr_t;

typedef ptrdiff_t br_ptrdiff_t;

/*
 * Boolean type
 */
typedef int br_boolean;

#define BR_TRUE		1
#define BR_FALSE	0

#define BR_BOOLEAN(a)	((a) != 0)

/*
 * 32 bit floating point
 */
typedef float br_float;

/**
 ** Compiler dependant type specifiers
 **/

/*
 * Microsoft Visual C++
 */
#if defined (_MSC_VER)

/* Handy for 64-bit porting. */

/* function' : pointer mismatch for actual parameter 'number' */
#pragma warning(error: 4022)
/* function' : actual parameter is not a pointer : parameter number */
//#pragma warning(error: 2172) // This is an actual error
/* operation' : different 'modifier' qualifiers */
#pragma warning(error: 4090)
/* variable' : pointer truncation from 'type' to 'type' */
#pragma warning(error: 4311)
/* operation' : conversion from 'type1' to 'type2' of greater size */
#pragma warning(error: 4312)
/* function' : 'format specifier' in format string conflicts with argument number of type 'type' */
#pragma warning(error: 4313)
/* type' : incompatible types - from 'type1' to 'type2' */
#pragma warning(error: 4133)

#pragma warning(error: 4024)
#pragma warning(error: 4477)
#pragma warning(error: 4047)
/*
 * Function qualifiers
 */
/*
 * Public entry point into library
 */
#define BR_PUBLIC		__cdecl
#define BR_PUBLIC_VA	__cdecl /* varargs version */

/*
 * Entry point exported to drivers via modules
 */
#define BR_EXPORT		__cdecl
#define BR_EXPORT_VA 	__cdecl /* varargs version */

/*
 * A driver object method
 */
#define BR_METHOD		__cdecl
#define BR_METHOD_VA	__cdecl	/* varargs version */

/*
 * A user defined callback function
 */
#define BR_CALLBACK		__cdecl

/*
 * A function with weak linkage
 */
#define BR_WEAK __weak

/*
 * A function using the cdecl calling convention. x86 only.
 */
#ifdef __i386__
#   define BR_CDECL __cdecl
#else
#   define BR_CDECL
#endif

#define BR_ASM_DATA
#define BR_ASM_CALL __cdecl
#define BR_ASM_CALLBACK __cdecl

#define BR_SUFFIX_HOST "-VISUALC"

#define BR_HAS_FAR	0

#define BR_ENDIAN_BIG		0
#define BR_ENDIAN_LITTLE	1

#define BR_PRINTF_ATTRIBUTE(fmt, args)
#define BR_SCANF_ATTRIBUTE(fmt, args)

//#warning "WHAT THE FUCK IS THIS"
#pragma warning(disable:4103)
#pragma pack(4)

/*
 * GNU C
 */
#elif defined (__GNUC__)

/*
 * Function qualifiers
 */
/*
 * Public entry point into library
 */
#define BR_PUBLIC		
#define BR_PUBLIC_VA	/* varargs version */

/*
 * Entry point exported to drivers via modules
 */
#define BR_EXPORT		
#define BR_EXPORT_VA 	/* varargs version */

/*
 * A driver object method
 */
#define BR_METHOD		
#define BR_METHOD_VA	/* varargs version */

/*
 * A user defined callback function
 */
#define BR_CALLBACK

/*
 * A function with weak linkage
 */
#define BR_WEAK __attribute__((weak))

/*
 * A function using the cdecl calling convention. x86 only.
 */
#ifdef __i386__
#   define BR_CDECL __attribute__((cdecl))
#else
#   define BR_CDECL
#endif

#define BR_ASM_DATA
#define BR_ASM_CALL 
#define BR_ASM_CALLBACK 

#define BR_SUFFIX_HOST "-GCC"

#define BR_HAS_FAR	0

#define BR_ENDIAN_BIG		0
#define BR_ENDIAN_LITTLE	1

#define BR_PRINTF_ATTRIBUTE(fmt, args) __attribute__((format(printf, fmt, args)))
#define BR_SCANF_ATTRIBUTE(fmt, args)  __attribute__((format(scanf,  fmt, args)))

#else
#	error "Unknown Compiler"
#endif

/*
 * Declare methods in C
 */
#define BR_CMETHOD_DECL(t,m)     	BR_METHOD _M_##t##_##m
#define BR_CMETHOD_VA_DECL(t,m)    	BR_METHOD _M_##t##_##m
#define BR_CMETHOD_PTR_DECL(t,m) 	(BR_METHOD * _##m)
#define BR_CMETHOD_VA_PTR_DECL(t,m) (BR_METHOD * _##m)

/*
 * Reference methods in C
 */

#define BR_CMETHOD(t,m)					(_M_##t##_##m)
#define BR_CMETHOD_REF(t,m) 		(_M_##t##_##m)
#define BR_CMETHOD_CALL(t,m,o) 		(((t *)(o))->dispatch->_##m)

/*
 * Backwards compatability
 */
#define BR_RESIDENT_ENTRY BR_EXPORT
#define BR_PUBLIC_ENTRY BR_PUBLIC

#ifdef _DEBUG
#	define DEBUG 1
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

/*
#if DEBUG
#	define BR_SUFFIX_DEBUG "-DEBUG"
#else
#	define BR_SUFFIX_DEBUG ""
#endif
*/

/*
 * Macros for producing banners & copyright messages
 */
#define BR_BANNER(title,year,revision)do {\
	static const char _revision[] = revision;\
	fprintf(stderr,title);\
	fwrite(_revision+10,1,sizeof(_revision)-12,stderr);\
	fprintf(stderr,"Copyright (C) " year " by Argonaut Technologies Limited\n");\
} while(0);

#if !defined(ADD_RCS_ID)
#define ADD_RCS_ID 0
#endif

#if ADD_RCS_ID
#define BR_RCS_ID(str) \
static const char rscid[] = str;
#else
#define BR_RCS_ID(str)
#endif

/*
 * Useful macro for sizing an array
 */
#define BR_ASIZE(a) (sizeof(a)/sizeof(*a))

/*
 * Make sure NULL is defined
 */
#ifndef NULL
#define NULL	0
#endif

#if defined(_WIN32)
#	ifndef __WIN_32__
#		define __WIN_32__
#	endif
#endif

#endif
