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

#include "util.h"
#include "wincompat.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */

long long libconfig_parse_integer(const char *s, int *ok)
{
  long long llval;
  char *endptr;
  int errsave = errno;
  errno = 0;
  llval = strtoll(s, &endptr, 0);	/* base 10 or base 8 */
  if(*endptr || errno)
  {
    errno = 0;
    *ok = 0;
    return(0);	/* parse error */
  }
  errno = errsave;

  *ok = 1;
  return(llval);
}

/* ------------------------------------------------------------------------- */

unsigned long long libconfig_parse_hex64(const char *s)
{
#ifdef __MINGW32__

  /* MinGW's strtoull() seems to be broken; it only returns the lower
   * 32 bits...
   */

  const char *p = s;
  unsigned long long val = 0;

  if(*p != '0')
    return(0);

  ++p;

  if(*p != 'x' && *p != 'X')
    return(0);

  for(++p; isxdigit(*p); ++p)
  {
    val <<= 4;
    val |= ((*p < 'A') ? (*p & 0xF) : (9 + (*p & 0x7)));
  }

  return(val);

#else /* ! __MINGW32__ */

  return(strtoull(s, NULL, 16));

#endif /* __MINGW32__ */
}

/* ------------------------------------------------------------------------- */

void libconfig_format_double(double val, int precision, int sci_ok, char *buf,
                             size_t buflen)
{
  const char *fmt = sci_ok ? "%.*g" : "%.*f";
  char *p, *q;

  snprintf(buf, buflen - 3, fmt, precision, val);

  /* Check for exponent. */
  p = strchr(buf, 'e');
  if(p) return;

  /* Check for decimal point. */
  p = strchr(buf, '.');
  if(!p)
  {
    /* No decimal point. Add trailing ".0". */
    strcat(buf, ".0");
  }
  else
  {
    /* Remove any excess trailing 0's after decimal point. */
    for(q = buf + strlen(buf) - 1; q > p + 1; --q)
    {
      if(*q == '0')
        *q = '\0';
      else
        break;
    }
  }
}

/* ------------------------------------------------------------------------- */
