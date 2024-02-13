/* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   Copyright (C) 2005-2020  Mark A Lindner

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

#ifndef __libconfig_strvec_h
#define __libconfig_strvec_h

#include <string.h>
#include <sys/types.h>

typedef struct
{
  const char **strings;
  const char **end;
  size_t length;
  size_t capacity;
} strvec_t;

extern void libconfig_strvec_append(strvec_t *vec, const char *s);

extern const char **libconfig_strvec_release(strvec_t *vec);

extern void libconfig_strvec_delete(const char * const *vec);

#endif /* __libconfig_strvec_h */
