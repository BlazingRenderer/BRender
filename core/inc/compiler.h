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

#if defined(__H2INC__)
typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed long    int32_t;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef signed long    intptr_t;
typedef unsigned long  uintptr_t;
typedef unsigned long  size_t;
typedef signed long    ssize_t;
typedef signed long    ptrdiff_t;
#else
#include <stdint.h>
#include <stddef.h>
#endif

/*
 * Fixed bitsize integers
 */
#if defined(__H2INC__)
typedef int32_t  br_int_64[2];
typedef uint32_t br_uint_64[2];
#else
typedef int64_t  br_int_64;
typedef uint64_t br_uint_64;
#endif

typedef int32_t  br_int_32;
typedef uint32_t br_uint_32;

typedef int16_t  br_int_16;
typedef uint16_t br_uint_16;

typedef int8_t  br_int_8;
typedef uint8_t br_uint_8;

/*
 * Generic size type (in case target environment does not have size_t)
 */
typedef size_t br_size_t;
#if defined(_MSC_VER) && !defined(__H2INC__)
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif

typedef intptr_t  br_intptr_t;
typedef uintptr_t br_uintptr_t;

typedef ptrdiff_t br_ptrdiff_t;

/*
 * Boolean type
 */
typedef int br_boolean;

#define BR_TRUE       1
#define BR_FALSE      0

#define BR_BOOLEAN(a) ((a) != 0)

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
#if defined(_MSC_VER)

/* Bot enough arguments for function-like macro invocation 'BR_PIXELMAP_MEMBERS_PREFIXED' */
#pragma warning(disable : 4003)

/* This function or variable may be unsafe. Consider using XXXXXX instead. */
#pragma warning(disable : 4996)

/*
 * Function qualifiers
 */
/*
 * Public entry point into library
 */
#define BR_PUBLIC    __cdecl
#define BR_PUBLIC_VA __cdecl /* varargs version */

/*
 * Entry point exported to drivers via modules
 */
#define BR_EXPORT    __cdecl
#define BR_EXPORT_VA __cdecl /* varargs version */

/*
 * A driver object method
 */
#define BR_METHOD    __cdecl
#define BR_METHOD_VA __cdecl /* varargs version */

/*
 * A user defined callback function
 */
#define BR_CALLBACK __cdecl

/*
 * A function using the cdecl calling convention. x86 only.
 */
#ifdef __i386__
#define BR_CDECL __cdecl
#else
#define BR_CDECL
#endif

#define BR_ASM_DATA
#define BR_ASM_CALL      __cdecl
#define BR_ASM_CALLBACK  __cdecl

#define BR_SUFFIX_HOST   "-VISUALC"

#define BR_HAS_FAR       0

#define BR_ENDIAN_BIG    0
#define BR_ENDIAN_LITTLE 1

#define BR_PRINTF_ATTRIBUTE(fmt, args)
#define BR_SCANF_ATTRIBUTE(fmt, args)

#define BR_STATIC_ASSERT(cond, msg)

/*
 * GNU C
 */
#elif defined(__GNUC__)

/*
 * Function qualifiers
 */
/*
 * Public entry point into library
 */
#define BR_PUBLIC
#define BR_PUBLIC_VA /* varargs version */

/*
 * Entry point exported to drivers via modules
 */
#define BR_EXPORT
#define BR_EXPORT_VA /* varargs version */

/*
 * A driver object method
 */
#define BR_METHOD
#define BR_METHOD_VA /* varargs version */

/*
 * A user defined callback function
 */
#define BR_CALLBACK

/*
 * A function using the cdecl calling convention. x86 only.
 */
#ifdef __i386__
#define BR_CDECL __attribute__((cdecl))
#else
#define BR_CDECL
#endif

#define BR_ASM_DATA
#define BR_ASM_CALL
#define BR_ASM_CALLBACK

#define BR_SUFFIX_HOST                 "-GCC"

#define BR_HAS_FAR                     0

#define BR_ENDIAN_BIG                  0
#define BR_ENDIAN_LITTLE               1

#define BR_PRINTF_ATTRIBUTE(fmt, args) __attribute__((format(printf, fmt, args)))
#define BR_SCANF_ATTRIBUTE(fmt, args)  __attribute__((format(scanf, fmt, args)))

#define BR_STATIC_ASSERT(cond, msg)    _Static_assert((cond), msg)

#else
#error "Unknown Compiler"
#endif

#define BR_STR(s) #s

#if defined(__H2INC__)
/*
 * Avoid some tokens that masm chokes on
 */
#define align  _align
#define seg    _seg
#define offset _offset
#define type   _type
#define size   _size
#define page   _page
#define mask   _mask
#define state  _state
#define ptr    _ptr
#define a      _a
#define b      _b
#define c      _c
#define width  _width
#define end    _end
#define out    _out
#define str    _str
#define ax     _ax
#define bx     _bx
#define cx     _cx
#define dx     _dx
#define si     _si
#define di     _di
#define bp     _bp
#define ip     _ip
#define sp     _sp
#define eax    _eax
#define ebx    _ebx
#define ecx    _ecx
#define edx    _edx
#define esi    _esi
#define edi    _edi
#define ebp    _ebp
#define eip    _eip
#define esp    _esp
#define al     _al
#define bl     _bl
#define cl     _cl
#define dl     _dl
#define ah     _ah
#define bh     _bh
#define ch     _ch
#define dh     _dh

#define es     _es
#define cs     _cs
#define ds     _ds
#define ss     _ss
#define fs     _fs
#define gs     _gs

#define low    _low
#define high   _high

/*
 * Supress compiler specific declarators
 */
#undef BR_CALLBACK
#undef BR_ASM_DATA
#undef BR_ASM_CALL
#undef BR_ASM_CALLBACK

#define BR_CALLBACK
#define BR_ASM_DATA
#define BR_ASM_CALL
#define BR_ASM_CALLBACK

#endif

/*
 * Declare methods in C
 */
#define BR_CMETHOD_DECL(t, m)        BR_METHOD _M_##t##_##m
#define BR_CMETHOD_VA_DECL(t, m)     BR_METHOD _M_##t##_##m
#define BR_CMETHOD_PTR_DECL(t, m)    (BR_METHOD * _##m)
#define BR_CMETHOD_VA_PTR_DECL(t, m) (BR_METHOD * _##m)

/*
 * Reference methods in C
 */

#define BR_CMETHOD(t, m)         (_M_##t##_##m)
#define BR_CMETHOD_REF(t, m)     (_M_##t##_##m)
#define BR_CMETHOD_CALL(t, m, o) (((t *)(o))->dispatch->_##m)
#define BR_CMETHOD_STR(t, m)     BR_STR(_M_##t##_##m)

/*
 * Backwards compatability
 */
#define BR_RESIDENT_ENTRY BR_EXPORT
#define BR_PUBLIC_ENTRY   BR_PUBLIC

#ifdef _DEBUG
#define DEBUG 1
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
#define BR_BANNER(title, year, revision)                                              \
    do {                                                                              \
        static const char _revision[] = revision;                                     \
        fprintf(stderr, title);                                                       \
        fwrite(_revision + 10, 1, sizeof(_revision) - 12, stderr);                    \
        fprintf(stderr, "Copyright (C) " year " by Argonaut Technologies Limited\n"); \
    } while(0);

#if !defined(ADD_RCS_ID)
#define ADD_RCS_ID 0
#endif

#if ADD_RCS_ID
#define BR_RCS_ID(str) static const char rscid[] = str;
#else
#define BR_RCS_ID(str)
#endif

/*
 * Useful macro for sizing an array
 */
#define BR_ASIZE(a) (sizeof(a) / sizeof(a[0]))

/*
 * Make sure NULL is defined
 */
#ifndef NULL
#define NULL 0
#endif

#if defined(_WIN32)
#ifndef __WIN_32__
#define __WIN_32__
#endif
#endif

#endif
