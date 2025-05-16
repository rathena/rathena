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

#include "strbuf.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>

#define STRING_BLOCK_SIZE 64

/* ------------------------------------------------------------------------- */

void libconfig_strbuf_ensure_capacity(strbuf_t *buf, size_t len)
{
  static const size_t mask = ~(STRING_BLOCK_SIZE - 1);

  size_t newlen = buf->length + len + 1; /* add 1 for NUL */
  if(newlen > buf->capacity)
  {
    buf->capacity = (newlen + (STRING_BLOCK_SIZE - 1)) & mask;
    buf->string = (char *)libconfig_realloc(buf->string, buf->capacity);
  }
}

/* ------------------------------------------------------------------------- */

char *libconfig_strbuf_release(strbuf_t *buf)
{
  char *r = buf->string;
  __zero(buf);
  return(r);
}

/* ------------------------------------------------------------------------- */

void libconfig_strbuf_append_string(strbuf_t *buf, const char *s)
{
  size_t len = strlen(s);
  libconfig_strbuf_ensure_capacity(buf, len);
  strcpy(buf->string + buf->length, s);
  buf->length += len;
}

/* ------------------------------------------------------------------------- */

void libconfig_strbuf_append_char(strbuf_t *buf, char c)
{
  libconfig_strbuf_ensure_capacity(buf, 1);
  *(buf->string + buf->length) = c;
  ++(buf->length);
  *(buf->string + buf->length) = '\0';
}

/* ------------------------------------------------------------------------- */
