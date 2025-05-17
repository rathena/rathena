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

#include "pathbuf.h"

#include <string.h>
#include <stdlib.h>

#define PATHBUF_BLOCK_SIZE 128

static void pathbuf_ensure_capacity(pathbuf_t *buf, size_t len)
{
  static const size_t mask = ~(PATHBUF_BLOCK_SIZE - 1);

  size_t newlen = buf->length + len + 1; /* add 1 for NUL */
  if(newlen > buf->capacity)
  {
    buf->capacity = (newlen + (PATHBUF_BLOCK_SIZE - 1)) & mask;
    buf->path = (char *)realloc(buf->path, buf->capacity);
  }
}

pathbuf_t *pathbuf_create(void)
{
  pathbuf_t *buf = (pathbuf_t *)malloc(sizeof(pathbuf_t));
  buf->path = (char *)malloc(PATHBUF_BLOCK_SIZE * sizeof(char));
  *(buf->path) = 0;
  buf->length = 0;
  buf->capacity = PATHBUF_BLOCK_SIZE;

  return(buf);
}

void pathbuf_append_path(pathbuf_t *buf, const char *path)
{
  pathbuf_append_path_len(buf, path, strlen(path));
}

extern void pathbuf_append_path_len(pathbuf_t *buf, const char *path, size_t len)
{
  int add_sep = 0;
  
  if(buf->length > 0)
    add_sep = *(buf->path + buf->length - 1) == '/' ? 0 : 1;
  
  pathbuf_ensure_capacity(buf, len + (add_sep ? 0 : 1));
  if(add_sep)
  {
    *(buf->path + buf->length) = '/';
    ++buf->length;
  }
  
  strncpy(buf->path + buf->length, path, len);
  buf->length += len;
  *(buf->path + buf->length) = '\0';
}

void pathbuf_remove_last_component(pathbuf_t *buf)
{
  char *p = strrchr(buf->path, '/');
  if(p > buf->path)
  {
    *p = '\0';
    buf->length = (size_t)(p - buf->path);
  }
}

void pathbuf_clear(pathbuf_t *buf)
{
  buf->length = 0;
  *(buf->path) = '\0';
}

void pathbuf_destroy(pathbuf_t *buf)
{
  free(buf->path);
  free(buf);
}
