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

#ifndef _NO_PROTOTYPES
#ifdef __cplusplus
extern "C" {
#endif

int BR_PUBLIC_ENTRY   BrMemCmp(const void *s1, const void *s2, br_size_t n);
void *BR_PUBLIC_ENTRY BrMemCpy(void *s1, const void *s2, br_size_t n);
void *BR_PUBLIC_ENTRY BrMemSet(void *s, int c, br_size_t n);

char *BR_PUBLIC_ENTRY     BrStrCat(char *s1, const char *s2);
int BR_PUBLIC_ENTRY       BrStrCmp(const char *s1, const char *s2);
int BR_PUBLIC_ENTRY       BrStrICmp(const char *s1, const char *s2);
char *BR_PUBLIC_ENTRY     BrStrCpy(char *s1, const char *s2);
br_size_t BR_PUBLIC_ENTRY BrStrLen(const char *s);
int BR_PUBLIC_ENTRY       BrStrNCmp(const char *s1, const char *s2, br_size_t n);
int BR_PUBLIC_ENTRY       BrStrNICmp(const char *s1, const char *s2, br_size_t n);
char *BR_PUBLIC_ENTRY     BrStrNCpy(char *s1, const char *s2, br_size_t n);

char *BR_PUBLIC_ENTRY BrStrChr(const char *s1, char c);
char *BR_PUBLIC_ENTRY BrStrRChr(const char *s1, char c);

void BR_PUBLIC_ENTRY BrAbort(void);

char *BR_PUBLIC_ENTRY BrGetEnv(const char *name);

float BR_PUBLIC_ENTRY         BrStrToF(const char *nptr, char **endptr);
double BR_PUBLIC_ENTRY        BrStrToD(const char *nptr, char **endptr);
long int BR_PUBLIC_ENTRY      BrStrToL(const char *nptr, char **endptr, int base);
unsigned long BR_PUBLIC_ENTRY BrStrToUL(const char *nptr, char **endptr, int base);

br_boolean BR_PUBLIC_ENTRY BrIsAlpha(int c);
br_boolean BR_PUBLIC_ENTRY BrIsDigit(int c);
br_boolean BR_PUBLIC_ENTRY BrIsSpace(int c);
br_boolean BR_PUBLIC_ENTRY BrIsPrint(int c);

br_int_32 BR_RESIDENT_ENTRY BrVSScanf(const char *str, const char *fmt, va_list args);
br_int_32 BR_RESIDENT_ENTRY BrVSprintf(char *buf, const char *fmt, va_list args);
br_int_32 BR_RESIDENT_ENTRY BrVSprintfN(char *buf, br_size_t buf_size, const char *fmt, va_list args);

#ifdef __cplusplus
};
#endif
#endif

#endif
