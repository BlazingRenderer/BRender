#ifndef __POSIXCOMPAT_H__
#define __POSIXCOMPAT_H__

#include <compiler.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
ssize_t getline(char **lineptr, size_t *n, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* __POSIXCOMPAT_H__ */