/*
 * Copyright (c) 1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brexcept.h 2.3 1996/10/03 11:35:10 sam Exp $
 * $Locker: $
 *
 * Simple exception handling
 */
#ifndef _BREXCEPT_H_
#define _BREXCEPT_H_

#include <setjmp.h>

/*
 * Exception types are the same as error types
 *
 * A value of zero is reserved
 */
typedef br_error br_exception;

/*
 * Exception handler - allocated as a resource 
 */
typedef struct br_exception_handler {
	struct br_exception_handler *prev;

	/*
	 * setjmp/longjmp context to throw to
	 */

	jmp_buf context;

} br_exception_handler;

/*
 * Public macros
 */
#define BrExceptionBegin(evp) (_BrExceptionValueFetch((br_exception)setjmp(_BrExceptionCatch()->context),(evp)))
#define BrExceptionEnd() _BrExceptionEnd()

#define BrExceptionThrow(et,ev) _BrExceptionThrow((br_exception)(ev),(void *)(ev))
#define BR_EXCEPTION_RESOURCE _BrExceptionResource()

#endif
