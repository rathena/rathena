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

#include "wincompat.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) \
  || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)

#include <errno.h>
#include <io.h>

int fsync(int fd)
{
  HANDLE h = (HANDLE)_get_osfhandle(fd);
  if(h == INVALID_HANDLE_VALUE)
  {
    errno = EBADF;
    return(-1);
  }

  if(!FlushFileBuffers(h))
  {
    DWORD err = GetLastError();
    switch(err)
    {
      case ERROR_ACCESS_DENIED:
        return(0);

      case ERROR_INVALID_HANDLE:
        errno = EINVAL;
        break;

      default:
        errno = EIO;
    }
    return(-1);
  }

  return(0);
}

#endif // WIN32 || WIN64
