/*
 * Copyright (c) 1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stdlib.c 1.1 1997/12/10 16:41:28 jon Exp $
 * $Locker: $
 *
 * Glue to stdlib functions
 */
#include "brender.h"

int BR_PUBLIC_ENTRY BrMemCmp(const void *s1, const void *s2, size_t n)
{
    const br_uint_8 *l=s1, *r=s2;
    for (; n && *l == *r; n--, l++, r++);
    return n ? *l-*r : 0;
}

void *BR_PUBLIC_ENTRY BrMemCpy(void *s1, const void *s2, size_t n)
{
    br_uint_8 *d = s1;
    const br_uint_8 *s = s2;
    for (; n; n--) *d++ = *s++;
    return s1;
}

void *BR_PUBLIC_ENTRY BrMemSet(void *s, int c, size_t n)
{
    br_uint_8 *p = s;
    for (; n; n--, p++) *p = c;
    return s;
}

char *BR_PUBLIC_ENTRY BrStrCat(char *s1, const char *s2)
{
    BrStrCpy(s1 + BrStrLen(s1), s2);
    return s1;
}

int BR_PUBLIC_ENTRY BrStrCmp(const char *s1, const char *s2)
{
    for (; *s1 == *s2 && *s1; s1++, s2++);
    return *(br_uint_8 *)s1 - *(br_uint_8 *)s2;
}

int BR_PUBLIC_ENTRY BrStrICmp(const char *s1, const char *s2)
{
    const br_uint_8 *l=(void *)s1, *r=(void *)s2;
    for (; *l && *r && (*l == *r || BrToLower(*l) == BrToLower(*r)); l++, r++);
    return BrToLower(*l) - BrToLower(*r);
}

char *BR_PUBLIC_ENTRY BrStrCpy(char *s1, const char *s2)
{
    for (; (*s1 = *s2); s2++, s1++);
    return s1;
}

char *BR_PUBLIC_ENTRY BrStpCpy(char *s1, const char *s2)
{
    return BrStrCpy(s1, s2) + BrStrLen(s2);
}

br_size_t BR_PUBLIC_ENTRY BrStrLen(const char *s)
{
    return __builtin_strlen(s);
}

int BR_PUBLIC_ENTRY BrStrNCmp(const char *s1, const char *s2, size_t n)
{
    return __builtin_strncmp(s1, s2, n);
}

int BR_PUBLIC_ENTRY BrStrNICmp(const char *s1, const char *s2, size_t n)
{
    const br_uint_8 *l=(void *)s1, *r=(void *)s2;
    if (!n--) return 0;
    for (; *l && *r && n && (*l == *r || BrToLower(*l) == BrToLower(*r)); l++, r++, n--);
    return BrToLower(*l) - BrToLower(*r);
}

char *BR_PUBLIC_ENTRY BrStrNCpy(char *s1, const char *s2, size_t n)
{
    for (; n && (*s1 = *s2); n--, s2++, s1++);
    BrMemSet(s1, 0, n);
    return s1;
}

char *BR_PUBLIC_ENTRY BrStrChr(const char *s1, char c)
{
    return BrMemChr(s1, c, BrStrLen(s1));
}

char *BR_PUBLIC_ENTRY BrStrRChr(const char *s1, char c)
{
    return BrMemRChr(s, c, BrStrLen(s));
}

void BR_PUBLIC_ENTRY BrAbort(void)
{
    abort();
}

const char *BR_PUBLIC_ENTRY BrGetEnv(const char *name)
{
    return NULL;
}

float BR_PUBLIC_ENTRY BrStrToF(const char *nptr, char **endptr)
{
    return (float)strtod(nptr, endptr);
}

double BR_PUBLIC_ENTRY BrStrToD(const char *nptr, char **endptr)
{
    return strtod(nptr, endptr);
}

long int BR_PUBLIC_ENTRY BrStrToL(const char *nptr, char **endptr, int base)
{
    return strtol(nptr, endptr, base);
}

long long int BR_PUBLIC_ENTRY BrStrToLL(const char *nptr, char **endptr, int base)
{
    return strtoll(nptr, endptr, base);
}

unsigned long BR_PUBLIC_ENTRY BrStrToUL(const char *nptr, char **endptr, int base)
{
    return strtoul(nptr, endptr, base);
}

unsigned long long BR_PUBLIC_ENTRY BrStrToULL(const char *nptr, char **endptr, int base)
{
    return strtoul(nptr, endptr, base);
}

int BR_PUBLIC_ENTRY BrAToI(const char *nptr)
{
    char    *end;
    long int result;

    result = BrStrToL(nptr, &end, 10);
    if(nptr == end)
        return 0;

    if(result > INT_MAX)
        return INT_MAX;

    if(result < INT_MIN)
        return INT_MIN;

    return (int)result;
}

long BR_PUBLIC_ENTRY BrAToL(const char *nptr)
{
    char *end;
    return BrStrToL(nptr, &end, 10);
}

long long BR_PUBLIC_ENTRY BrAToLL(const char *nptr)
{
    char *end;
    return BrStrToLL(nptr, &end, 10);
}

float BR_PUBLIC_ENTRY BrAToF(const char *nptr)
{
    char *end;
    return BrStrToF(nptr, &end);
}

br_boolean BR_PUBLIC_ENTRY BrIsAlpha(int c)
{
    return isalpha(c);
}

br_boolean BR_PUBLIC_ENTRY BrIsDigit(int c)
{
    return isdigit(c);
}

br_boolean BR_PUBLIC_ENTRY BrIsSpace(int c)
{
    return isspace(c);
}

br_boolean BR_PUBLIC_ENTRY BrIsPrint(int c)
{
    return isprint(c);
}

br_int_32 BR_PUBLIC_ENTRY BrVSprintf(char *buf, const char *fmt, va_list args)
{
    return vsprintf(buf, fmt, args);
}

br_int_32 BR_PUBLIC_ENTRY BrVSprintfN(char *buf, br_size_t buf_size, const char *fmt, va_list args)
{
    unsigned int n;
    char         tmp[512];

    n = vsprintf(tmp, fmt, args);

    if(n > buf_size - 1) {
        n = buf_size - 1;
    }

    BrStrNCpy(buf, tmp, n);
    buf[n] = '\0';

    return n;
}

br_int_32 BR_PUBLIC_ENTRY BrVSScanf(const char *buf, const char *fmt, va_list args)
{
    return vsscanf(buf, fmt, args);
}
