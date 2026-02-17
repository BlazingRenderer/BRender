/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brstdlib.h 1.1 1997/12/10 16:41:05 jon Exp $
 * $Locker: $
 *
 * Wrappers around standard C library functions that are provided by the STD library
 */

#ifndef _BRSTDLIB_H_
#define _BRSTDLIB_H_

#if !defined(__H2INC__)
#include <stdarg.h>
#endif

#ifdef BR_FREESTANDING
#define PRIu8 "hu"
#define PRId8 "hd"
#define PRIu16 "hu"
#define PRId16 "hd"
#define PRIu32 "lu"
#define PRId32 "ld"
#define PRIu64 "llu"
#define PRId64 "lld"
#define PRIuPTR "lu"
#define PRIdPTR "ld"
#define SCNxPTR "lx"
#define SCNdPTR "ld"
#define INFINITY __builtin_inf()
#define sin(n) __builtin_sin(n)
#define sinf(n) __builtin_sinf(n)
#define cos(n) __builtin_cos(n)
#define cosf(n) __builtin_cosf(n)
#define tan(n) __builtin_tan(n)
#define tanf(n) __builtin_tanf(n)
#define fabs(n) __builtin_fabs(n)
#define fabsf(n) __builtin_fabsf(n)
#define fmod(a, b) __builtin_fmod(a, b)
#define fmodf(a, b) __builtin_fmodf(a, b)
#define atan2(a, b) __builtin_atan2(a, b)
#define atan2f(a, b) __builtin_atan2f(a, b)
#define sqrt(n) __builtin_sqrt(n)
#define sqrtf(n) __builtin_sqrtf(n)
#endif

#ifndef _NO_PROTOTYPES
#ifdef __cplusplus
extern "C" {
#endif

int BR_PUBLIC_ENTRY   BrMemCmp(const void *s1, const void *s2, br_size_t n);
void *BR_PUBLIC_ENTRY BrMemCpy(void *s1, const void *s2, br_size_t n);
void *BR_PUBLIC_ENTRY BrMemSet(void *s, int c, br_size_t n);
void *BR_PUBLIC_ENTRY BrMemChr(const void *m, int c, size_t n);
void *BR_PUBLIC_ENTRY BrMemRChr(const void *m, int c, size_t n);

char *BR_PUBLIC_ENTRY     BrStrCat(char *s1, const char *s2);
int BR_PUBLIC_ENTRY       BrStrCmp(const char *s1, const char *s2);
int BR_PUBLIC_ENTRY       BrStrICmp(const char *s1, const char *s2);
char *BR_PUBLIC_ENTRY     BrStrCpy(char *s1, const char *s2);
char *BR_PUBLIC_ENTRY     BrStpCpy(char *s1, const char *s2);
br_size_t BR_PUBLIC_ENTRY BrStrLen(const char *s);
int BR_PUBLIC_ENTRY       BrStrNCmp(const char *s1, const char *s2, br_size_t n);
int BR_PUBLIC_ENTRY       BrStrNICmp(const char *s1, const char *s2, br_size_t n);
char *BR_PUBLIC_ENTRY     BrStrNCpy(char *s1, const char *s2, br_size_t n);

char *BR_PUBLIC_ENTRY BrStrChr(const char *s1, char c);
char *BR_PUBLIC_ENTRY BrStrRChr(const char *s1, char c);

BR_NORETURN void BR_PUBLIC_ENTRY BrAbort(void);

const char *BR_PUBLIC_ENTRY BrGetEnv(const char *name);

float BR_PUBLIC_ENTRY              BrStrToF(const char *nptr, char **endptr);
double BR_PUBLIC_ENTRY             BrStrToD(const char *nptr, char **endptr);
long int BR_PUBLIC_ENTRY           BrStrToL(const char *nptr, char **endptr, int base);
long long int BR_PUBLIC_ENTRY      BrStrToLL(const char *nptr, char **endptr, int base);
unsigned long BR_PUBLIC_ENTRY      BrStrToUL(const char *nptr, char **endptr, int base);
unsigned long long BR_PUBLIC_ENTRY BrStrToULL(const char *nptr, char **endptr, int base);

int BR_PUBLIC_ENTRY       BrAToI(const char *nptr);
long BR_PUBLIC_ENTRY      BrAToL(const char *nptr);
long long BR_PUBLIC_ENTRY BrAToLL(const char *nptr);
float BR_PUBLIC_ENTRY     BrAToF(const char *nptr);

br_boolean BR_PUBLIC_ENTRY BrIsAlpha(int c);
br_boolean BR_PUBLIC_ENTRY BrIsDigit(int c);
br_boolean BR_PUBLIC_ENTRY BrIsSpace(int c);
br_boolean BR_PUBLIC_ENTRY BrIsPrint(int c);
br_boolean BR_PUBLIC_ENTRY BrIsUpper(int c);

int BR_PUBLIC_ENTRY BrToLower(int c);

br_int_32 BR_RESIDENT_ENTRY BrVSScanf(const char *str, const char *fmt, va_list args);
br_int_32 BR_RESIDENT_ENTRY BrVSprintf(char *buf, const char *fmt, va_list args);
br_int_32 BR_RESIDENT_ENTRY BrVSprintfN(char *buf, br_size_t buf_size, const char *fmt, va_list args);

#ifdef __cplusplus
};
#endif
#endif

#endif
