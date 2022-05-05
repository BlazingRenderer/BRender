#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "posixcompat.h"


extern "C" ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
    static const size_t growIncrement = 80;
    char *pTmp = NULL;

    char c = 0;
    size_t index = 0;

    if(!stream || !lineptr || !n)
    {
        errno = EINVAL;
        return -1;
    }

    if(!(*lineptr) || (*n < 2))
    {
        if(!(*lineptr = (char *)malloc(growIncrement)))
        {
            errno = ENOMEM;
            return -1;
        }

        *n += growIncrement;
    }

    for(;;)
    {
        if((c = fgetc(stream)) == EOF)
            break;

        while(index > (*n - 2))
        {
            if(!(pTmp = (char *)malloc(*n + growIncrement)))
            {
                free(*lineptr);
                errno = ENOMEM;
                return -1;
            }

            memcpy(pTmp, *lineptr, *n);
            free(*lineptr);
            *lineptr = pTmp;
            *n += growIncrement;

        }

        (*lineptr)[index++] = c;

        if(c == delim)
            break;
    }

    if(index)
        (*lineptr)[index] = 0;
    else if(c == EOF)
        return -1;

    return index;
}

extern "C" ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    return getdelim(lineptr, n, '\n', stream);
}