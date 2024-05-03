// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef NULLPO_HPP
#define NULLPO_HPP

#include "cbasetypes.hpp"

#define NLP_MARK __FILE__, __LINE__, __func__

// enabled by default on debug builds
#if defined(DEBUG) && !defined(NULLPO_CHECK)
#define NULLPO_CHECK
#endif

#if defined(NULLPO_CHECK)

/**
 * Macros used to check for nullptr pointer and output that information.
 */

/**
 * Return 0 if pointer is not found.
 * @param t: Pointer to check
 * @return 0 if t is nullptr
 */
#define nullpo_ret(t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {return(0);}

/**
 * Return void if pointer is not found.
 * @param t: Pointer to check
 * @return void if t is nullptr
 */
#define nullpo_retv(t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {return;}

/**
 * Return the given value if pointer is not found.
 * @param ret: Value to return
 * @param t: Pointer to check
 * @return ret value
 */
#define nullpo_retr(ret, t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {return(ret);}

/**
 * Break out of the loop/switch if pointer is not found.
 * @param t: Pointer to check
 */
#define nullpo_retb(t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {break;}

// Different C compilers uses different argument formats
#if __STDC_VERSION__ >= 199901L || defined(_MSC_VER)
/* C99 standard */
/**
 * Return 0 and display additional information if pointer is not found.
 * @param t: Pointer to check
 * @param fmt: Pass to vprintf, Format and arguments such as description
 * @return 0 if t is nullptr
 */
#define nullpo_ret_f(t, fmt, ...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), __VA_ARGS__)) {return(0);}

/**
 * Return void and display additional information if pointer is not found.
 * @param t: Pointer to check
 * @param fmt: Pass to vprintf, Format and arguments such as description
 * @return void if t is nullptr
 */
#define nullpo_retv_f(t, fmt, ...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), __VA_ARGS__)) {return;}

/**
 * Return the given value and display additional information if pointer is not found.
 * @param t: Pointer to check
 * @param fmt: Pass to vprintf, Format and arguments such as description
 * @return ret value
 */
#define nullpo_retr_f(ret, t, fmt, ...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), __VA_ARGS__)) {return(ret);}

/**
 * Break out of the loop/switch and display additional information if pointer is not found.
 * @param t: Pointer to check
 * @param fmt: Pass to vprintf, Format and arguments such as description
 */
#define nullpo_retb_f(t, fmt, ...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), __VA_ARGS__)) {break;}

#elif __GNUC__ >= 2
/* For GCC */
/**
 * Return 0 and display additional information if pointer is not found.
 * @param t: Pointer to check
 * @param fmt: Pass to vprintf, Format and arguments such as description
 * @return 0 if t is nullptr
 */
#define nullpo_ret_f(t, fmt, args...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), ## args)) {return(0);}

/**
 * Return void and display additional information if pointer is not found.
 * @param t: Pointer to check
 * @param fmt: Pass to vprintf, Format and arguments such as description
 * @return void if t is nullptr
 */
#define nullpo_retv_f(t, fmt, args...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), ## args)) {return;}

/**
 * Return the given value and display additional information if pointer is not found.
 * @param t: Pointer to check
 * @param fmt: Pass to vprintf, Format and arguments such as description
 * @return ret value
 */
#define nullpo_retr_f(ret, t, fmt, args...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), ## args)) {return(ret);}

/**
 * Break out of the loop/switch and display additional information if pointer is not found.
 * @param t: Pointer to check
 * @param fmt: Pass to vprintf, Format and arguments such as description
 */
#define nullpo_retb_f(t, fmt, args...) \
	if (nullpo_chk_f(NLP_MARK, (void *)(t), (fmt), ## args)) {break;}

#else
/* Otherwise... */
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
#if __STDC_VERSION__ >= 199901L || defined(_MSC_VER)
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

/**
 * Check for nullptr pointer and output information.
 * @param file: __FILE__
 * @param line: __LINE__
 * @param func: __func__ (name of the function) [NLP_MARK]
 * @param target: Target to check
 * @return 0 on success or 1 on nullptr
 */
int nullpo_chk(const char *file, int line, const char *func, const void *target);

/**
 * Check for nullptr pointer and output detailed information.
 * @param file: __FILE__
 * @param line: __LINE__
 * @param func: __func__ (name of the function) [NLP_MARK]
 * @param target: Target to check
 * @param fmt: Passed to vprintf
 * @return 0 on success or 1 on nullptr
 */
int nullpo_chk_f(const char *file, int line, const char *func, const void *target,
                 const char *fmt, ...)
                 __attribute__((format(printf,5,6)));

/**
 * Display information of the code that cause this function to trigger.
 * @param file: __FILE__
 * @param line: __LINE__
 * @param func: __func__ (name of the function) [NLP_MARK]
 * @param target: Target to check
 */
void nullpo_info(const char *file, int line, const char *func);

/**
 * Check for nullptr pointer and output detailed information.
 * @param file: __FILE__
 * @param line: __LINE__
 * @param func: __func__ (name of the function) [NLP_MARK]
 * @param target: Target to check
 * @param fmt: Passed to vprintf
 */
void nullpo_info_f(const char *file, int line, const char *func, 
                   const char *fmt, ...)
                   __attribute__((format(printf,4,5)));

#endif /* NULLPO_HPP */
