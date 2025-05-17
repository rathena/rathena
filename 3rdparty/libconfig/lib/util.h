/* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   Copyright (C) 2005-2025  Mark A Lindner

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

#include <string.h>
#include <stdint.h>
#include <sys/types.h>

extern void libconfig_set_fatal_error_func(void (*func)(const char *));

extern void libconfig_fatal_error(const char *message);

extern void *libconfig_malloc(size_t size);
extern void *libconfig_calloc(size_t nmemb, size_t size);
extern void *libconfig_realloc(void *ptr, size_t size);

#define __new(T) (T *)libconfig_calloc(1, sizeof(T)) /* zeroed */
#define __delete(P) free((void *)(P))
#define __zero(P) memset((void *)(P), 0, sizeof(*P))

extern long long libconfig_parse_integer(const char *s, int long_ok, int *ok);
extern unsigned long long libconfig_parse_hex64(const char *s, int long_ok,
                                                int *ok);
extern unsigned long long libconfig_parse_bin64(const char *s, int long_ok,
                                                int *ok);

extern void libconfig_format_double(double val, int precision, int sci_ok,
                                    char *buf, size_t buflen);
extern void libconfig_format_bin(int64_t val, char *buf, size_t buflen);
