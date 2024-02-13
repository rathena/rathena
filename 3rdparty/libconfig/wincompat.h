/* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   Copyright (C) 2005-2018  Mark A Lindner

   This file is part of libconfig.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

#ifndef __wincompat_h
#define __wincompat_h

#include <limits.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) \
  || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)

/* Prevent warnings about redefined malloc/free in generated code. */
#ifndef _STDLIB_H
#define _STDLIB_H
#endif

#include <malloc.h>

#ifdef _MSC_VER
#pragma warning (disable: 4996)
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define fileno _fileno

#if _MSC_VER <= 1800
#define snprintf  _snprintf
#endif

#if !defined(__MINGW32__) && _MSC_VER < 1800
#define atoll     _atoi64
#define strtoull  _strtoui64
#define strtoll   _strtoi64
#endif

#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#endif

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) \
  || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) \
  || defined(__MINGW32__))

#define INT64_FMT "%I64d"
#define UINT64_FMT "%I64u"

#define INT64_HEX_FMT "%I64X"

#define FILE_SEPARATOR "\\"

#else /* defined(WIN32) || defined(__MINGW32__) */

#define INT64_FMT "%lld"
#define UINT64_FMT "%llu"

#define INT64_HEX_FMT "%llX"

#define FILE_SEPARATOR "/"

#endif /* defined(WIN32) || defined(__MINGW32__) */

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) \
  || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)) \
  && ! defined(__MINGW32__)

#define INT64_CONST(I)  (I ## i64)
#define UINT64_CONST(I) (I ## Ui64)

#ifndef INT_MAX
#define INT_MAX (2147483647)
#endif

#ifndef INT_MIN
#define INT_MIN (-2147483647-1)
#endif

#include <Shlwapi.h>
#define IS_RELATIVE_PATH(P) \
  (PathIsRelativeA(P))

extern int fsync(int fd);

#else /* defined(WIN32/WIN64) && ! defined(__MINGW32__) */

#define INT64_CONST(I)  (I ## LL)
#define UINT64_CONST(I) (I ## ULL)

#define IS_RELATIVE_PATH(P) \
  ((P)[0] != '/')

#include <unistd.h> /* for fsync() */

#endif /* defined(WIN32/WIN64) && ! defined(__MINGW32__) */

#endif /* __wincompat_h */
