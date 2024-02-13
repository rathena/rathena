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

#include "strvec.h"
#include "util.h"

#include <stdlib.h>

#define CHUNK_SIZE 32

/* ------------------------------------------------------------------------- */

void libconfig_strvec_append(strvec_t *vec, const char *s)
{
  if(vec->length == vec->capacity)
  {
    vec->capacity += CHUNK_SIZE;
    vec->strings = (const char **)realloc(
        (void *)vec->strings,
        (vec->capacity + 1) * sizeof(const char *));
    vec->end = vec->strings + vec->length;
  }

  *(vec->end) = s;
  ++(vec->end);
  ++(vec->length);
}

/* ------------------------------------------------------------------------- */

const char **libconfig_strvec_release(strvec_t *vec)
{
  const char **r = vec->strings;
  if(r)
    *(vec->end) = NULL;

  __zero(vec);
  return(r);
}

/* ------------------------------------------------------------------------- */

void libconfig_strvec_delete(const char *const *vec)
{
  const char *const *p;

  if(!vec) return;

  for(p = vec; *p; ++p)
    __delete(*p);

  __delete(vec);
}

/* ------------------------------------------------------------------------- */
