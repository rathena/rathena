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

#ifndef __libconfig_scanctx_h
#define __libconfig_scanctx_h

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "libconfig.h"
#include "strbuf.h"
#include "strvec.h"

#define MAX_INCLUDE_DEPTH 10

struct include_stack_frame
{
  /*
   * Member strings are not owned; they are pointers into scan_context's
   * filenames vector.
   */
  const char **files;
  const char **current_file;
  FILE *current_stream;
  void *parent_buffer;
};

struct scan_context
{
  config_t *config;
  const char *top_filename;
  struct include_stack_frame include_stack[MAX_INCLUDE_DEPTH];
  int stack_depth;
  strbuf_t string;
  strvec_t filenames;
};

extern void libconfig_scanctx_init(struct scan_context *ctx,
                                   const char *top_filename);
extern const char **libconfig_scanctx_cleanup(struct scan_context *ctx);

/*
 * Pushes a new frame onto the include stack, and returns an open stream to the
 * first file in the include list, if any.
 *
 * ctx - The scan context
 * prev_buffer - The current input buffer, to be restored when this frame is
 * popped
 * path - The string argument to the @include directive, to be expanded into a
 * list of zero or more filenames using the function ctx->config->include_fn
 * error - A pointer at which to store a static error message, if any.
 *
 * On success, the new frame will be pushed and the stream to the first file
 * will be returned.
 *
 * On failure, the frame will not be pushed and NULL will be returned. If
 * *error is NULL, it means there are no files in the list. Otherwise, it
 * points to an error and parsing should be aborted.
 */
extern FILE *libconfig_scanctx_push_include(struct scan_context *ctx,
                                            void *prev_buffer,
                                            const char *path,
                                            const char **error);

/*
 * Returns the next include file in the current include stack frame.
 *
 * Returns NULL on failure or if there are no more files left in the current
 * frame. If there was an error, sets *error.
 */
extern FILE *libconfig_scanctx_next_include_file(struct scan_context *ctx,
                                                 const char **error);

/*
 * Pops a frame off the include stack.
 */
extern void *libconfig_scanctx_pop_include(struct scan_context *ctx);

#define libconfig_scanctx_append_string(C, S) \
  libconfig_strbuf_append_string(&((C)->string), (S))

#define libconfig_scanctx_append_char(C, X) \
  libconfig_strbuf_append_char(&((C)->string), (X))

extern char *libconfig_scanctx_take_string(struct scan_context *ctx);

extern const char *libconfig_scanctx_current_filename(struct scan_context *ctx);

#endif /* __libconfig_scanctx_h */
