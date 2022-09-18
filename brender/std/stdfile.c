/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stdfile.c 2.7 1996/11/20 18:07:52 STEVEW Exp $
 * $Locker: $
 *
 * Default file handler that uses <stdio.h>
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "brender.h"
#include "host.h"
#include "brassert.h"


BR_RCS_ID("$Id: stdfile.c 2.7 1996/11/20 18:07:52 STEVEW Exp $")



#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static wchar_t *BrStdioToWinWchar(const char *utf8name, wchar_t *buf, br_size_t count)
{
	if(utf8name == NULL)
		return NULL;

	br_size_t inlen = BrStrLen(utf8name);

	int x = MultiByteToWideChar(CP_UTF8, 0, utf8name, (int)inlen, buf, (int)count);
	if(x == 0)
		return NULL;

	buf[x] = L'\0';
	return buf;
}
#endif

static FILE *BrStdioFopenUtf8(const char *utf8Name, const char *mode)
{
#ifdef _WIN32
	static wchar_t buf[4096];

	wchar_t *name = BrStdioToWinWchar(utf8Name, buf, BR_ASIZE(buf));
	if(name == NULL)
		return NULL;

	br_size_t modelen = BrStrLen(mode);

	const wchar_t *lmode = alloca(modelen * sizeof(wchar_t));
	if(BrStdioToWinWchar(mode, lmode, modelen) == NULL)
		return NULL;

	return _wfopen(name, lmode);
#else
	return fopen(utf8Name, mode);
#endif
}


/*
 * Access functions for stdio
 */
static br_uint_32 BR_CALLBACK BrStdioAttributes(void)
{
	return 
		BR_FS_ATTR_READABLE   | 
		BR_FS_ATTR_WRITEABLE  |
		BR_FS_ATTR_HAS_TEXT   |
		BR_FS_ATTR_HAS_BINARY |
		BR_FS_ATTR_HAS_ADVANCE;
}

/*
 * Open a file for reading
 *
 * Use BRENDER_PATH to locate the file if necessary
 *
 * Having found the file, invoke a supplied callback, (if present)
 * to find out if the file is text, binary, or unknown
 *
 * Text mode files are primarily used for debugging but it can be
 * useful to allow hand editting of input data
 *
 * Return a void * file handle ('FILE *' cast to 'void *') or NULL
 * if open failed
 */
static void * BR_CALLBACK BrStdioOpenRead(const char *name, br_size_t n_magics,
		br_mode_test_cbfn *identify,
		int *mode_result)
{
	FILE *fh;
	char *br_path, config_path[512];
   	char try_name[512],*cp;
	br_uint_8 magics[BR_MAX_FILE_MAGICS];
	int open_mode = BR_FS_MODE_BINARY;

	/*
	 * Try the current directory
	 */
	strncpy(try_name,name,BR_ASIZE(try_name)-1);

	if((fh = BrStdioFopenUtf8(try_name,"rb")) == NULL) {

		/*
		 * If that fails, and if a drive or a
	     * directory were specified, don't search along path
		 */
		if(strchr(name,':') || strchr(name,'/') || strchr(name,'\\'))
			return NULL;

		/*
		 * For each element of the path, if it exists
		 */

      if ( BrSystemConfigQueryString( BRT_BRENDER_PATH_STR, config_path,
                           BR_ASIZE(config_path) ) != BRE_OK )
         return NULL ;

		br_path = config_path;

		while(*br_path) {
			/*
			 * Take characters until next seperator or terminator
			 */
			cp = try_name;

			while(*br_path != ';' && *br_path != '\0')
				*cp++ = *br_path++;

			if(*br_path == ';')
				br_path++;

			/*
			 * Add a directory seperator if none
			 */
			if(cp != try_name && (*(cp-1) != ':' && *(cp-1) != '/' &&*(cp-1) != '\\'))
				*cp++ = '/';

			strcpy(cp,name);

			if((fh = BrStdioFopenUtf8(try_name,"rb")) != NULL)
				break;
		}

		if(fh == NULL)
			return NULL;
	}

	if(mode_result)
		 open_mode = *mode_result;

	if(n_magics != 0) {
		/*
		 * Now have an open file, try and grab the first bytes from it
		 */
		if(fread(magics, 1, n_magics,fh) != n_magics) {
			/*
			 * Could not read all the required data, close and punt
			 */
			fclose(fh);
			return NULL;
		}

		/*
		 * Try and identify the file
		 */
		if(identify)
			open_mode = identify(magics, n_magics);

		if(mode_result)
			*mode_result = open_mode;
	}

	/*
	 * Reopen file with it's new identity (or abandon if unknown identity)
	 */
	fclose(fh);

	switch(open_mode) {
	case BR_FS_MODE_TEXT:
		fh = BrStdioFopenUtf8(try_name,"r");
		break;

	case BR_FS_MODE_BINARY:
		fh = BrStdioFopenUtf8(try_name,"rb");
		break;

	case BR_FS_MODE_UNKNOWN:
		fh = NULL;
		break;

	default:
		BR_ERROR1("BrStdFileOpenRead: invalid open_mode %d",open_mode);
	}

	return fh;
}


/*
 * Open a file for writing, overwrites any existing file of the same name
 *
 * Return a void * file handle ('FILE *' cast to 'void *') or NULL
 * if open failed
 */
static void * BR_CALLBACK BrStdioOpenWrite(const char *name, int mode)
{
	FILE *fh = NULL;

	fh = BrStdioFopenUtf8(name,(mode == BR_FS_MODE_TEXT)?"w":"wb");

	return fh;
}

/*
 * Close an open file
 */
static void BR_CALLBACK BrStdioClose(void *f)
{
	fclose(f);
}

/*
 * Test EOF
 */
static int BR_CALLBACK BrStdioEof(void *f)
{
	return feof((FILE *)f);
}

/*
 * Read one character from file
 */
static int BR_CALLBACK BrStdioGetChar(void *f)
{
	return getc((FILE *)f);
}


/*
 * Write one character to file
 */
static void BR_CALLBACK BrStdioPutChar(int c, void *f)
{
	fputc(c,(FILE *)f);
}

/*
 * Read a block from a file
 */
static br_size_t BR_CALLBACK BrStdioRead(void *buf, br_size_t size, br_size_t n, void *f)
{
	return fread(buf,size,n,(FILE *)f);
}

/*
 * Write a block to a file
 */
static br_size_t BR_CALLBACK BrStdioWrite(const void *buf, br_size_t size, br_size_t n, void *f)
{
	return fwrite(buf,size,n,(FILE *)f);
}

/*
 * Read a line of text from stdin and trim any terminators
 *
 * Return length of line
 */
static br_size_t BR_CALLBACK BrStdioGetLine(char *buf, br_size_t buf_len, void * f)
{
	br_size_t l;

	ASSERT( f );
	ASSERT(buf_len < INT_MAX);

	if(fgets(buf,(int)buf_len,(FILE *)f) == NULL)
		return 0;

	l = strlen(buf);

	if(l > 0 && buf[l-1] == '\n')
		buf[--l] = '\0';
		
	return l;
}

/*
 * Write a line to text file, followed by newline
 */
static void BR_CALLBACK BrStdioPutLine(const char *buf, void * f)
{
	fputs(buf,(FILE *)f);
	fputc('\n',(FILE *)f);
}

/*
 * Advance N bytes through a binary stream
 */
static void BR_CALLBACK BrStdioAdvance(br_size_t count, void *f)
{
	fseek(f,(long int)count,SEEK_CUR);
}

static void * BR_CALLBACK BrStdioLoad(void *res, const char *name, br_size_t *size)
{
	FILE *fh = NULL;
	void *data = NULL;
	long fsize = -1;

	fh = BrStdioOpenRead(name, 0, NULL, NULL);
	if(fh == NULL)
		return NULL;

	if(fseek(fh, 0, SEEK_END) < 0)
		goto done;

	if((fsize = ftell(fh)) < 0)
		goto done;

	if(fseek(fh, 0, SEEK_SET) < 0)
		goto done;

	data = BrResAllocate(res, (br_size_t)fsize, BR_MEMORY_APPLICATION);
	if(fread(data, (br_size_t)fsize, 1, fh) != 1)
	{
		BrResFree(data);
		data = NULL;
		goto done;
	}

	*size = (br_size_t)fsize;
done:
	BrStdioClose(fh);
	return data;
}

/*
 * Old version of BrStdioLoad(), can work on files without seek/tell.
 * Keeping around for reference.
 */
static void * BR_CALLBACK BrStdioLoadOld(void *res, const char *name, br_size_t *size)
{
	br_size_t currentSize;
	void *brf = NULL;
	void *buffer = NULL;

	currentSize = 64;

	for(brf = NULL, buffer = NULL;; currentSize += 64)
	{
		if(brf != NULL)
			BrFileClose(brf);

		if(buffer != NULL)
			BrResFree(buffer);

		if(!(brf = BrFileOpenRead(name, 0, NULL, NULL)))
			return NULL;

		buffer = BrResAllocate(res, currentSize, BR_MEMORY_APPLICATION);
		*size = BrFileRead(buffer, 1, currentSize, brf);

		if(BrFileEof(brf))
			break;
	}

	BrFileClose(brf);
	return buffer;
}

/*
 * Filesystem structure
 */
br_filesystem BrStdioFilesystem = {
	.identifier	= "Standard IO",
	
	.attributes	= BrStdioAttributes,
	.open_read	= BrStdioOpenRead,
	.open_write	= BrStdioOpenWrite,
	.close		= BrStdioClose,
	.eof		= BrStdioEof,

	.getchr		= BrStdioGetChar,
	.putchr		= BrStdioPutChar,

	.read		= BrStdioRead,
	.write		= BrStdioWrite,

	.getline	= BrStdioGetLine,
	.putline	= BrStdioPutLine,

	.advance	= BrStdioAdvance,

	.load		= BrStdioLoad,
};

/*
 * Override global variable s.t. the default filesystem will be stdio
 */
br_filesystem * BR_ASM_DATA _BrDefaultFilesystem = &BrStdioFilesystem;
