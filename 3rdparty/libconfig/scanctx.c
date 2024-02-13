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

#include "scanctx.h"
#include "strvec.h"
#include "wincompat.h"
#include "util.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */

static const char *err_bad_include = "cannot open include file";
static const char *err_include_too_deep = "include file nesting too deep";

/* ------------------------------------------------------------------------- */

void libconfig_scanctx_init(struct scan_context *ctx, const char *top_filename)
{
  __zero(ctx);
  if(top_filename)
  {
    ctx->top_filename = strdup(top_filename);
    libconfig_strvec_append(&(ctx->filenames), ctx->top_filename);
  }
}

/* ------------------------------------------------------------------------- */

const char **libconfig_scanctx_cleanup(struct scan_context *ctx)
{
  int i;

  for(i = 0; i < ctx->stack_depth; ++i)
  {
    struct include_stack_frame *frame = &(ctx->include_stack[i]);

    if(frame->current_stream)
      fclose(frame->current_stream);

    __delete(frame->files);
  }

  __delete(libconfig_strbuf_release(&(ctx->string)));

  return(libconfig_strvec_release(&(ctx->filenames)));
}

/* ------------------------------------------------------------------------- */

FILE *libconfig_scanctx_push_include(struct scan_context *ctx, void *prev_buffer,
                                     const char *path, const char **error)
{
  struct include_stack_frame *frame;
  const char **files = NULL, **f;
  FILE *fp;

  if(ctx->stack_depth == MAX_INCLUDE_DEPTH)
  {
    *error = err_include_too_deep;
    return(NULL);
  }

  *error = NULL;

  if(ctx->config->include_fn)
    files = ctx->config->include_fn(ctx->config, ctx->config->include_dir,
                                    path, error);

  if(*error || !files)
  {
    libconfig_strvec_delete(files);
    return(NULL);
  }

  if(!*files)
  {
    libconfig_strvec_delete(files);
    return(NULL);
  }

  frame = &(ctx->include_stack[ctx->stack_depth]);

  for(f = files; *f; ++f)
    libconfig_strvec_append(&(ctx->filenames), *f);

  frame->files = files;
  frame->current_file = NULL;
  frame->current_stream = NULL;
  frame->parent_buffer = prev_buffer;
  ++(ctx->stack_depth);

  fp = libconfig_scanctx_next_include_file(ctx, error);
  if(!fp)
    (void)libconfig_scanctx_pop_include(ctx);

  return(fp);
}

/* ------------------------------------------------------------------------- */

FILE *libconfig_scanctx_next_include_file(struct scan_context *ctx,
                                          const char **error)
{
  struct include_stack_frame *include_frame;

  *error = NULL;

  if(ctx->stack_depth == 0)
    return(NULL);

  include_frame = &(ctx->include_stack[ctx->stack_depth - 1]);

  if(include_frame->current_file)
    ++(include_frame->current_file);
  else
    include_frame->current_file = include_frame->files;

  if(include_frame->current_stream)
  {
    fclose(include_frame->current_stream);
    include_frame->current_stream = NULL;
  }

  if(!*(include_frame->current_file))
    return(NULL);

  include_frame->current_stream = fopen(*(include_frame->current_file), "rt");
  if(!include_frame->current_stream)
    *error = err_bad_include;

  return(include_frame->current_stream);
}

/* ------------------------------------------------------------------------- */

void *libconfig_scanctx_pop_include(struct scan_context *ctx)
{
  struct include_stack_frame *frame;

  if(ctx->stack_depth == 0)
    return(NULL); /* stack underflow */

  frame = &(ctx->include_stack[--(ctx->stack_depth)]);

  __delete(frame->files);
  frame->files = NULL;

  if(frame->current_stream)
  {
    fclose(frame->current_stream);
    frame->current_stream = NULL;
  }

  return(frame->parent_buffer);
}

/* ------------------------------------------------------------------------- */

char *libconfig_scanctx_take_string(struct scan_context *ctx)
{
  char *r = libconfig_strbuf_release(&(ctx->string));

  return(r ? r : strdup(""));
}

/* ------------------------------------------------------------------------- */

const char *libconfig_scanctx_current_filename(struct scan_context *ctx)
{
  if(ctx->stack_depth > 0)
    return(*(ctx->include_stack[ctx->stack_depth - 1].current_file));

  return(ctx->top_filename);
}

/* ------------------------------------------------------------------------- */
