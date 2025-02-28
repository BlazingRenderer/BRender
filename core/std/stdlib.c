/*
 * Copyright (c) 1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stdlib.c 1.1 1997/12/10 16:41:28 jon Exp $
 * $Locker: $
 *
 * Glue to stdlib functions
 */
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <SDL3/SDL.h>

#include "brender.h"

int BR_PUBLIC_ENTRY BrMemCmp(const void *s1, const void *s2, size_t n)
{
    return SDL_memcmp(s1, s2, n);
}

void *BR_PUBLIC_ENTRY BrMemCpy(void *s1, const void *s2, size_t n)
{
    return SDL_memcpy(s1, s2, n);
}

void *BR_PUBLIC_ENTRY BrMemSet(void *s, int c, size_t n)
{
    return SDL_memset(s, c, n);
}

char *BR_PUBLIC_ENTRY BrStrCat(char *s1, const char *s2)
{
    return strcat(s1, s2);
}

int BR_PUBLIC_ENTRY BrStrCmp(const char *s1, const char *s2)
{
    return SDL_strcmp(s1, s2);
}

int BR_PUBLIC_ENTRY BrStrICmp(const char *s1, const char *s2)
{
    return SDL_strcasecmp(s1, s2);
}

char *BR_PUBLIC_ENTRY BrStrCpy(char *s1, const char *s2)
{
    return strcpy(s1, s2);
}

char *BR_PUBLIC_ENTRY BrStpCpy(char *s1, const char *s2)
{
    return strcpy(s1, s2) + strlen(s2);
}

br_size_t BR_PUBLIC_ENTRY BrStrLen(const char *s)
{
    return SDL_strlen(s);
}

int BR_PUBLIC_ENTRY BrStrNCmp(const char *s1, const char *s2, size_t n)
{
    return SDL_strncmp(s1, s2, n);
}

int BR_PUBLIC_ENTRY BrStrNICmp(const char *s1, const char *s2, size_t n)
{
    return SDL_strncasecmp(s1, s2, n);
}

char *BR_PUBLIC_ENTRY BrStrNCpy(char *s1, const char *s2, size_t n)
{
    return strncpy(s1, s2, n);
}

char *BR_PUBLIC_ENTRY BrStrChr(const char *s1, char c)
{
    return SDL_strchr(s1, c);
}

char *BR_PUBLIC_ENTRY BrStrRChr(const char *s1, char c)
{
    return SDL_strrchr(s1, c);
}

void BR_PUBLIC_ENTRY BrAbort(void)
{
    abort();
}

const char *BR_PUBLIC_ENTRY BrGetEnv(const char *name)
{
    return SDL_getenv(name);
}

float BR_PUBLIC_ENTRY BrStrToF(const char *nptr, char **endptr)
{
    return (float)SDL_strtod(nptr, endptr);
}

double BR_PUBLIC_ENTRY BrStrToD(const char *nptr, char **endptr)
{
    return SDL_strtod(nptr, endptr);
}

long int BR_PUBLIC_ENTRY BrStrToL(const char *nptr, char **endptr, int base)
{
    return SDL_strtol(nptr, endptr, base);
}

long long int BR_PUBLIC_ENTRY BrStrToLL(const char *nptr, char **endptr, int base)
{
    return SDL_strtoll(nptr, endptr, base);
}

unsigned long BR_PUBLIC_ENTRY BrStrToUL(const char *nptr, char **endptr, int base)
{
    return SDL_strtoul(nptr, endptr, base);
}

unsigned long long BR_PUBLIC_ENTRY BrStrToULL(const char *nptr, char **endptr, int base)
{
    return SDL_strtoul(nptr, endptr, base);
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
    return SDL_isalpha(c);
}

br_boolean BR_PUBLIC_ENTRY BrIsDigit(int c)
{
    return SDL_isdigit(c);
}

br_boolean BR_PUBLIC_ENTRY BrIsSpace(int c)
{
    return SDL_isspace(c);
}

br_boolean BR_PUBLIC_ENTRY BrIsPrint(int c)
{
    return SDL_isprint(c);
}

br_int_32 BR_PUBLIC_ENTRY BrVSprintf(char *buf, const char *fmt, va_list args)
{
    return SDL_vsnprintf(buf, ~(size_t)0, fmt, args);
}

br_int_32 BR_PUBLIC_ENTRY BrVSprintfN(char *buf, br_size_t buf_size, const char *fmt, va_list args)
{
    return SDL_vsnprintf(buf, buf_size, fmt, args);
}

br_int_32 BR_PUBLIC_ENTRY BrVSScanf(const char *buf, const char *fmt, va_list args)
{
    return SDL_vsscanf(buf, fmt, args);
}
