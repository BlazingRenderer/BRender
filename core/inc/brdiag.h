/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brdiag.h 1.1 1997/12/10 16:41:15 jon Exp $
 * $Locker: $
 *
 * Brender's interface to diagnostic handling
 */

#ifndef _BRDIAG_H_
#define _BRDIAG_H_

/*
 * Instance of an error handler
 */
/**
 * \brief An application defined call-back function that is called upon a non-serious, unexpected
 *        failure, with a message describing the warning.
 *
 * \param message Pointer to zero terminated character string describing the warning.
 *
 * \pre The diagnostic handler has been setup (BrDiagHandlerSet()). BRender has not necessarily
 *      completed initialisation. In a recoverable state.
 *
 * \post Optionally inform user of warning. Return.
 *
 * \remark This function is primarily intended for debugging and testing purposes. BRender does not
 *         currently generate warnings.
 *
 * \sa br_diag_failure_cbfn.
 *
 * \par Example
 * \code{.c}
 * See stddiag.c for examples of diagnostic handler functions.
 * \endcode
 */
typedef void BR_CALLBACK br_diag_warning_cbfn(const char *message);
/**
 * \brief An application defined call-back function that is called upon a serious, unexpected error
 *        or failure, with a descriptive message.
 *
 * Remember, a release version of your software should never fail.
 *
 * \param message Pointer to zero terminated character string describing the failure.
 *
 * \pre The diagnostic handler has been setup (BrDiagHandlerSet()). BRender has not necessarily
 *      completed initialisation. In an unknown state. A failure has occurred. BRender is not
 *      expecting the function to return.
 *
 * \post Behaviour is up to the application, but the following procedure can be taken as a
 *       suggestion. Set a failure condition flag (to detect escalation). Optionally, immediately
 *       inform user of failure. Re-establish a known state. Perform diagnostics of hardware and
 *       software, inform user of any diagnosed faults (optionally, also of original failure). End
 *       failure condition, and resume (do not return).
 *
 * \remark Avoid allocating memory in your failure call-back function, or you may need special care
 *         in handling out-of-memory conditions. BRender's default failure call-back function does
 *         not allocate memory.
 *
 * \sa br_diag_warning_cbfn.
 *
 * \par Example
 * \code{.c}
 * See stddiag.c for examples of diagnostic handler functions.
 * \endcode
 */
typedef void BR_CALLBACK br_diag_failure_cbfn(const char *message);

/**
 * \brief This structure represents the definition of a diagnostic handler.
 *
 * All BRender's diagnostics are handled by just two functions, which can be specified by the
 * programmer. This is essential in cases where stdout and stderr (as used by the standard C library
 * functions) are not available (or not suitable).
 */
typedef struct br_diaghandler {
    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * A string constant is recommended.
     */
    const char           *identifier;
    /**
     * \brief This is a pointer to the warning delivery function (See br_diag_warning_cbfn).
     *
     * This is invoked by the warning macros (See Diagnostic Support, page 65).
     */
    br_diag_warning_cbfn *warning;
    /**
     * \brief This is a pointer to the failure delivery function (See br_diag_failure_cbfn).
     *
     * This is invoked by the failure, fatal and assertion macros (See Diagnostic Support).
     */
    br_diag_failure_cbfn *failure;
} br_diaghandler;

/*
 * For backwards compatibility
 */
typedef struct br_errorhandler {
    const char           *identifier;
    br_diag_warning_cbfn *message;
    br_diag_failure_cbfn *error;
} br_errorhandler;

/**
 ** Macros for diagnostic generation
 **/

/*
 * Report message and exit - should not return to application
 */
#define BR_FAILURE(s)                    BrFailure(s)
#define BR_FAILURE0(s)                   BrFailure(s)
#define BR_FAILURE1(s, a)                BrFailure(s, a)
#define BR_FAILURE2(s, a, b)             BrFailure(s, a, b)
#define BR_FAILURE3(s, a, b, c)          BrFailure(s, a, b, c)
#define BR_FAILURE4(s, a, b, c, d)       BrFailure(s, a, b, c, d)
#define BR_FAILURE5(s, a, b, c, d, e)    BrFailure(s, a, b, c, d, e)
#define BR_FAILURE6(s, a, b, c, d, e, f) BrFailure(s, a, b, c, d, e, f)

/*
 * Report message and continue
 */
#define BR_WARNING(s)                    BrWarning(s)
#define BR_WARNING0(s)                   BrWarning(s)
#define BR_WARNING1(s, a)                BrWarning(s, a)
#define BR_WARNING2(s, a, b)             BrWarning(s, a, b)
#define BR_WARNING3(s, a, b, c)          BrWarning(s, a, b, c)
#define BR_WARNING4(s, a, b, c, d)       BrWarning(s, a, b, c, d)
#define BR_WARNING5(s, a, b, c, d, e)    BrWarning(s, a, b, c, d, e)
#define BR_WARNING6(s, a, b, c, d, e, f) BrWarning(s, a, b, c, d, e, f)

/*
 * Report message and exit, including source file and line number
 */
#define BR_FATAL(s)                    BrFatal(__FILE__, __LINE__, s)
#define BR_FATAL0(s)                   BrFatal(__FILE__, __LINE__, s)
#define BR_FATAL1(s, a)                BrFatal(__FILE__, __LINE__, s, a)
#define BR_FATAL2(s, a, b)             BrFatal(__FILE__, __LINE__, s, a, b)
#define BR_FATAL3(s, a, b, c)          BrFatal(__FILE__, __LINE__, s, a, b, c)
#define BR_FATAL4(s, a, b, c, d)       BrFatal(__FILE__, __LINE__, s, a, b, c, d)
#define BR_FATAL5(s, a, b, c, d, e)    BrFatal(__FILE__, __LINE__, s, a, b, c, d, e)
#define BR_FATAL6(s, a, b, c, d, e, f) BrFatal(__FILE__, __LINE__, s, a, b, c, d, e, f)

/*
 * Backwards compatibility
 */
#define BR_ERROR(s)                    BrFailure(s)
#define BR_ERROR0(s)                   BrFailure(s)
#define BR_ERROR1(s, a)                BrFailure(s, a)
#define BR_ERROR2(s, a, b)             BrFailure(s, a, b)
#define BR_ERROR3(s, a, b, c)          BrFailure(s, a, b, c)
#define BR_ERROR4(s, a, b, c, d)       BrFailure(s, a, b, c, d)
#define BR_ERROR5(s, a, b, c, d, e)    BrFailure(s, a, b, c, d, e)
#define BR_ERROR6(s, a, b, c, d, e, f) BrFailure(s, a, b, c, d, e, f)

#endif
