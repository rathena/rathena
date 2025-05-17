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

#ifndef __pathbuf_h
#define __pathbuf_h

#include <string.h>
#include <sys/types.h>

typedef struct
{
  char *path;
  size_t length;
  size_t capacity;
} pathbuf_t;

extern pathbuf_t *pathbuf_create(void);

extern void pathbuf_append_path(pathbuf_t *buf, const char *path);

extern void pathbuf_append_path_len(pathbuf_t *buf, const char *path, size_t len);

extern void pathbuf_remove_last_component(pathbuf_t *buf);

#define pathbuf_get_path(BUF) \
  ((BUF)->path)

#define pathbuf_get_length(BUF) \
  ((BUF)->length)

extern void pathbuf_clear(pathbuf_t *buf);

extern void pathbuf_destroy(pathbuf_t *buf);

#endif /* __pathbuf_h */
