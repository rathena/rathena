// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _NULLPO_H_
#define _NULLPO_H_


#include "cbasetypes.h"

#define NLP_MARK __FILE__, __LINE__, __func__

// enabled by default on debug builds
#if defined(DEBUG) && !defined(NULLPO_CHECK)
#define NULLPO_CHECK
#endif

/*----------------------------------------------------------------------------
 * Macros
 *----------------------------------------------------------------------------
 */
/*======================================
 * Check for NULL pointer and output information
 *--------------------------------------
 * nullpo_ret(t)
 *   Returns 0 if <t> is NULL.
 * [Argument]
 *  t       Target to check
 *--------------------------------------
 * nullpo_retv(t)
 *   Returns nothing (void) if <t> is NULL.
 * [Argument]
 *  t       Target to check
 *--------------------------------------
 * nullpo_retr(ret, t)
 * Returns <ret> if <t> is NULL.
 * [Arguments]
 *  ret     Value to return;
 *  t       Target to check
 *--------------------------------------
 * nullpo_ret_f(t, fmt, ...)
 *   For displaying additional information
 *   Returns 0 if t is NULL.
 * [Arguments]
 *  t       Target to check
 *  fmt ... Passed to vprintf
 *    Format and arguments such as description
 *--------------------------------------
 * nullpo_retv_f(t, fmt, ...)
 *   For displaying additional information
 *   Returns nothing (void) if <t> is NULL.
 * [Arguments]
 *  t       Target to check
 *  fmt ... Passed to vprintf
 *    Format and arguments such as description
 *--------------------------------------
 * nullpo_retr_f(ret, t, fmt, ...)
 *   For displaying additional information
 *   Returns <ret> if <t> is NULL.
 * [Arguments]
 *  ret     return(ret);
 *  t       Target to check
 *  fmt ... Passed to vprintf
 *    Format and arguments such as description
 *--------------------------------------
 */

#if defined(NULLPO_CHECK)

#define nullpo_ret(t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {return(0);}

#define nullpo_retv(t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {return;}

#define nullpo_retr(ret, t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {return(ret);}

#define nullpo_retb(t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {break;}

// Different C compilers uses different argument formats
#if __STDC_VERSION__ >= 199901L
/* C99 standard */
#define nullpo_ret_f(t, fmt, ...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), __VA_ARGS__)) {return(0);}

#define nullpo_retv_f(t, fmt, ...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), __VA_ARGS__)) {return;}

#define nullpo_retr_f(ret, t, fmt, ...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), __VA_ARGS__)) {return(ret);}

#define nullpo_retb_f(t, fmt, ...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), __VA_ARGS__)) {break;}

#elif __GNUC__ >= 2
/* For GCC */
#define nullpo_ret_f(t, fmt, args...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), ## args)) {return(0);}

#define nullpo_retv_f(t, fmt, args...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), ## args)) {return;}

#define nullpo_retr_f(ret, t, fmt, args...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), ## args)) {return(ret);}

#define nullpo_retb_f(t, fmt, args...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), ## args)) {break;}

#else

/* Otherwise... orz */

#endif

#else /* NULLPO_CHECK */
/* No Nullpo check */

// if((t)){;}
// Do nothing if Nullpo check is disabled

#define nullpo_ret(t) (void)(t)
#define nullpo_retv(t) (void)(t)
#define nullpo_retr(ret, t) (void)(t)
#define nullpo_retb(t) (void)(t)

// Different C compilers uses different argument formats
#if __STDC_VERSION__ >= 199901L
/* C99 standard */
#define nullpo_ret_f(t, fmt, ...) (void)(t)
#define nullpo_retv_f(t, fmt, ...) (void)(t)
#define nullpo_retr_f(ret, t, fmt, ...) (void)(t)
#define nullpo_retb_f(t, fmt, ...) (void)(t)

#elif __GNUC__ >= 2
/* For GCC */
#define nullpo_ret_f(t, fmt, args...) (void)(t)
#define nullpo_retv_f(t, fmt, args...) (void)(t)
#define nullpo_retr_f(ret, t, fmt, args...) (void)(t)
#define nullpo_retb_f(t, fmt, args...) (void)(t)

#else
/* Otherwise... orz */
#endif

#endif /* NULLPO_CHECK */

/*----------------------------------------------------------------------------
 * Functions
 *----------------------------------------------------------------------------
 */
/*======================================
 * nullpo_chk
 *   Check for null and output information
 * [Arguments]
 *  file    __FILE__
 *  line    __LINE__
 *  func    __func__ (name of the function)
 *    You may pass NLP_MARK
 *  target  Target to check
 * [Return values]
 *  0 OK
 *  1 NULL
 *--------------------------------------
 */
int nullpo_chk(const char *file, int line, const char *func, const void *target);


/*======================================
 * nullpo_chk_f
 *   Check for NULL pointer and output detailed information
 * [Arguments]
 *  file    __FILE__
 *  line    __LINE__
 *  func    __func__ (name of the function)
 *    You may pass NLP_MARK
 *  target  Target to check
 *  fmt ... Passed to vprintf
 *    Format and arguments such as description
 * [Return values]
 *  0 OK
 *  1 NULL
 *--------------------------------------
 */
int nullpo_chk_f(const char *file, int line, const char *func, const void *target,
                 const char *fmt, ...)
                 __attribute__((format(printf,5,6)));


/*======================================
 * nullpo_info
 *   Display information of the code that cause this function to trigger
 * [Arguments]
 *  file    __FILE__
 *  line    __LINE__
 *  func    __func__ (name of the function)
 *    You may pass NLP_MARK
 *--------------------------------------
 */
void nullpo_info(const char *file, int line, const char *func);


/*======================================
 * nullpo_info_f
 *   Check for NULL pointer with additional
 *   information.
 * [Arguments]
 *  file    __FILE__
 *  line    __LINE__
 *  func    __func__ (name of the function)
 *    You may pass NLP_MARK
 *  fmt ... Passed to vprintf
 *    Format and arguments such as description
 *--------------------------------------
 */
void nullpo_info_f(const char *file, int line, const char *func, 
                   const char *fmt, ...)
                   __attribute__((format(printf,4,5)));


#endif /* _NULLPO_H_ */
